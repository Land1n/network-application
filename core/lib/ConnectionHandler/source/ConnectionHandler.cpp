//
// Created by ivan on 07.03.2026.
//
#include "ConnectionHandler.hpp"
#include <algorithm>

#include "sdrlogger/sdrlogger.h"

std::vector<ConnectedSocket> &ConnectionHandler::getSockets() {
    return connected_sockets_;
}

std::shared_ptr<tcp::acceptor> ConnectionHandler::getAcceptor() {
    return acceptor_;
}

std::shared_ptr<tcp::socket> ConnectionHandler::getSocket() {
    return socket_client_;
};


bool ConnectionHandler::isInWork() const {
    return isWork.load();
}

void ConnectionHandler::start() {
    if (!isWork.load()) {
        isWork.store(true);
        std::string type_name;
        if (this->type == ConnectionHandlerType::Client)
            type_name = "Client";
        else if (this->type == ConnectionHandlerType::Server)
            type_name = "Server";
        else
            type_name = "Unknown";
        logger("INFO") << "ConnectionHandler [start()] : Start " << type_name << "\n";
    } else {
        stop();
    }
}

void ConnectionHandler::stop() {
    if (!isWork.load()) return;
    isWork.store(false);

    logger("DEBUG") << "ConnectionHandler [stop()] : Stopping " << (type == ConnectionHandlerType::Server
                                                                        ? "Server": "Client") << "\n";
    if (acceptor_thread != nullptr)
        acceptor_thread->join();

    if (connection_thread != nullptr)
        connection_thread->join();

    if (acceptor_ != nullptr && acceptor_->is_open()) {
        acceptor_->cancel();
        acceptor_->close();
        logger("DEBUG") << "ConnectionHandler [stop()] : Acceptor closed\n";
        acceptor_.reset();
    } else if (acceptor_ != nullptr) {
        acceptor_.reset();
    }

    if (type == ConnectionHandlerType::Server) {
        logger("DEBUG") << "ConnectionHandler [stop()] : Connected_sockets.size() = " << std::to_string(
            connected_sockets_.size()) << "\n";
        while (connected_sockets_.size() != 0) {
            disconnected(connected_sockets_.back(), true);
        }
        connected_sockets_.clear();
    }

    if (socket_client_ != nullptr && socket_client_->is_open()) {
        disconnect();
        socket_client_.reset();
    }
    if (socket_client_ != nullptr)
        socket_client_.reset();


    logger("INFO") << "ConnectionHandler [stop()] : Stop " << (type == ConnectionHandlerType::Server
                                                                   ? "Server"
                                                                   : "Client") << "\n";
}

ConnectionHandler::ConnectionHandler(std::string &address, int port, ConnectionHandlerType type, bool INFO)
    : address(address), port(port), type(type) {
    logger.init5Levels();
    if (INFO) {
        logger.setLogLevel("ERROR");
    }
    start();
    if (type == ConnectionHandlerType::Server)
        acceptor_ = createAcceptor();
}

ConnectionHandler::~ConnectionHandler() {
    stop();
}

std::shared_ptr<tcp::acceptor> ConnectionHandler::createAcceptor() {
    logger("DEBUG") << "ConnectionHandler [createAcceptor()] : Creating acceptor..." << "\n";

    if (type == ConnectionHandlerType::Client) {
        logger("ERROR") << "ConnectionHandler [createAcceptor()] : ConnectionHandlerType = Client" << "\n";
        return nullptr;
    }

    if (!isWork.load()) {
        logger("ERROR") << "ConnectionHandler [createAcceptor()] : isWork = " << isWork.load() << "\n";
        return nullptr;
    }
    std::shared_ptr<tcp::acceptor> acceptor = std::make_shared<tcp::acceptor>(io_context);
    acceptor->open(tcp::v4());
    boost::system::error_code error_code;
    acceptor->set_option(tcp::acceptor::reuse_address(true));
    acceptor->bind(tcp::endpoint(boost::asio::ip::address::from_string(address), port), error_code);
    if (!error_code || error_code == boost::asio::error::address_in_use) {
        if (error_code == boost::asio::error::address_in_use)
            logger("WARN") << "ConnectionHandler [createAcceptor()] : Address_in_use" << "\n";
        logger("DEBUG") << "ConnectionHandler [createAcceptor()] : Create acceptor successful" << "\n";
        logger("INFO") << "ConnectionHandler [createAcceptor()] : Acceptor on " << address << ":" << port << "\n";
    } else {
        logger("ERROR") << "ConnectionHandler [createAcceptor()] : Create acceptor fail" << "\n";
        logger("ERROR") << "ConnectionHandler [createAcceptor()] : " << error_code.what() << "\n";
        stop();
        return nullptr;
    }
    if (acceptor==nullptr)
        logger("ERROR") << "ConnectionHandler [createAcceptor()] : acceptor==nullptr\n";
    return acceptor;
}

std::shared_ptr<tcp::socket> ConnectionHandler::createSocket() {
    return std::make_shared<tcp::socket>(io_context);
}

std::shared_ptr<tcp::socket> ConnectionHandler::accept(bool blocking) {
    if (type == ConnectionHandlerType::Client) {
        logger("ERROR") << "ConnectionHandler [accept()] : ConnectionHandlerType = Client" << "\n";
        return nullptr;
    }

    if (!isWork.load()) {
        logger("ERROR") << "ConnectionHandler [accept()] : isWork = " << isWork.load() << "\n";
        return nullptr;
    }

    auto socket = std::make_shared<tcp::socket>(io_context);
    boost::system::error_code ec;
    acceptor_->non_blocking(!blocking);
    acceptor_->accept(*socket, ec);
    if (ec == boost::asio::error::would_block) {
        return nullptr;
    } else if (ec) {
        logger("ERROR") << "ConnectionHandler [accept()] : " << ec.message() << "\n";
        return nullptr;
    }
    auto connected_socket = ConnectedSocket(socket);
    logger("INFO") << "ConnectionHandler [accept()] : Accept socket " << connected_socket.getAddressAndPort() << "\n";
    connected_sockets_.push_back(connected_socket);

    return socket;
}

bool ConnectionHandler::disconnected(ConnectedSocket &connected_socket, bool delete_socket) {
    boost::system::error_code ec;
    try {
        connected_socket.ptr->close(ec);
        if (!ec) {
            logger("INFO") << "ConnectionHandler [disconnected()] : Disconnected socket " << connected_socket.
                    getAddressAndPort() << "\n";
            if (delete_socket) {
                auto it = std::find_if(connected_sockets_.begin(), connected_sockets_.end(),
                                       [socket = connected_socket.ptr](const ConnectedSocket &cs) {
                                           return cs.ptr == socket;
                                       });
                if (it != connected_sockets_.end()) {
                    connected_sockets_.erase(it);
                }
                logger("DEBUG") << "ConnectionHandler [disconnected()] : Delete socket from connected_sockets\n";
            }
            return true;
        } else {
            logger("WARN") << "ConnectionHandler [disconnected()] : Disconnected socket " << connected_socket.
                    getAddressAndPort() << "\n";
            logger("WARN") << "ConnectionHandler [disconnected()] : " << ec.message() << "\n";
            return false;
        }
    } catch (std::exception &e) {
        logger("ERROR") << "ConnectionHandler [disconnected()] : " << e.what() << "\n";
    }
    return false;
}

void ConnectionHandler::disconnect() {
    if (type == ConnectionHandlerType::Server) {
        logger("ERROR") << "ConnectionHandler [disconnect()] : ConnectionHandlerType = Server" << "\n";
    }
    boost::system::error_code ec;

    socket_client_->close(ec);
    if (ec) {
        logger("ERROR") << "ConnectionHandler [disconnect()] : Disconnect socket " << address << ":" << port << "\n";
    }
    logger("INFO") << "ConnectionHandler [disconnect()] : Disconnect socket " << address << ":" << port << "\n";
}

std::shared_ptr<tcp::socket> ConnectionHandler::connect() {
    if (type == ConnectionHandlerType::Server) {
        logger("ERROR") << "ConnectionHandler [connect()] : ConnectionHandlerType = Server" << "\n";
        return nullptr;
    }
    boost::system::error_code ec;
    socket_client_ = createSocket();
    socket_client_->connect(tcp::endpoint(boost::asio::ip::address::from_string(address), port), ec);
    if (ec) {
        logger("ERROR") << "ConnectionHandler [connect()] : Connect socket " << address << ":" << port << "\n";
        isWork.store(false);
        return nullptr;
    } else {
        logger("INFO") << "ConnectionHandler [connect()] : Connect socket " << socket_client_->remote_endpoint().
                address().to_string() << ":" << socket_client_->remote_endpoint().port() << "\n";

        auto self = shared_from_this();
        connection_thread = std::make_shared<std::thread>([self]() {
            self->logger("DEBUG") << "ConnectionHandler [is_connected()] : Start check connection...\n";
            while (self->isWork.load()) {
                if (!self->is_connected(self->socket_client_)) {
                    self->logger("DEBUG") << "ConnectionHandler [is_connected()] : Socket " << self->address << ":" <<
                            self->port << " is_connected() = false" << "\n";
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            self->logger("DEBUG") << "ConnectionHandler [is_connected()] : Stop check connection\n";
        });
        if (task_) {
            task_(socket_client_);
        }
        return socket_client_;
    }
    return nullptr;
}

bool ConnectionHandler::is_connected(std::shared_ptr<tcp::socket> sock) {
    sock->non_blocking(true);
    boost::system::error_code ec;
    char c;
    sock->read_some(boost::asio::buffer(&c, 1), ec);
    sock->non_blocking(false);
    if (!ec || ec == boost::asio::error::would_block) {
        return true;
    }
    return false;
}

void ConnectionHandler::setTaskSocket(std::function<void(std::shared_ptr<tcp::socket>)> task) {
    task_ = std::move(task);
}

void ConnectionHandler::listen() {
    if (type == ConnectionHandlerType::Client) {
        logger("ERROR") << "ConnectionHandler [listen()] : ConnectionHandlerType = Client\n";
        return;
    }
    if (!isWork.load()) {
        logger("ERROR") << "ConnectionHandler [listen()] : isWork = false\n";
        return;
    }
    boost::system::error_code ec;
    acceptor_->listen();
    auto self = shared_from_this();
    acceptor_thread = std::make_shared<std::thread>([self]() {
            self->logger("INFO") << "ConnectionHandler [listen()] : Start listen connection...\n";
            while (self->isWork.load()) {
                auto socket = self->accept(false);
                if (socket) {
                    if (self->task_) {
                        self->task_(socket);
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            self->logger("INFO") << "ConnectionHandler [listen()] : Stopped listen connection\n";
    });

    connection_thread = std::make_shared<std::thread>([self]() {
        try {
            self->logger("DEBUG") << "ConnectionHandler [is_connected()] : Start check connection...\n";
            while (self->isWork.load()) {
                if (!self->connected_sockets_.empty()) {
                    for (int i = 0; i < self->connected_sockets_.size(); i++)
                        if (!self->is_connected(self->connected_sockets_[i].ptr)) {
                            ConnectedSocket cs(self->connected_sockets_[i].ptr);
                            self->logger("DEBUG") << "ConnectionHandler [is_connected()] : Socket " << cs.
                                    getAddressAndPort() << " is_connected() = false" << "\n";
                            self->disconnected(cs);
                            self->connected_sockets_.erase(self->connected_sockets_.begin() + i);
                            break;
                        }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            self->logger("DEBUG") << "ConnectionHandler [is_connected()] : Stop check connection\n";
        } catch (std::exception &e) {
            self->logger("ERROR") << "ConnectionHandler [is_connected()] : ConnectionHandler " << e.what() << "\n";
        }
    });
}
