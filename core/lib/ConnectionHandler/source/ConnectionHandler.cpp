//
// Created by ivan on 07.03.2026.
//
#include "ConnectionHandler.hpp"
#include <algorithm>
#include "ThreadPool.hpp"
#include "sdrlogger/sdrlogger.h"

std::vector<ConnectedSocket> &ConnectionHandler::getSockets() {
    if (type == ConnectionHandlerType::Server)
        return connected_sockets_;
    else {
        connected_sockets_.clear();
        return connected_sockets_;
    }
}

std::shared_ptr<tcp::acceptor> ConnectionHandler::getAcceptor() {
    return acceptor_;
}

std::shared_ptr<tcp::socket> ConnectionHandler::getSocket() {
    return socket_client_;
};

void ConnectionHandler::start() {
    if (!isWork->load()) {
        isWork->store(true);
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
    if (!isWork->load()) return;
    isWork->store(false);


    if (acceptor_ != nullptr && acceptor_->is_open()) {
        acceptor_->cancel();
        acceptor_->close();
        logger("DEBUG") << "ConnectionHandler [stop()] : Acceptor closed\n";
    }  else if (acceptor_ != nullptr) {
        acceptor_.reset();
    }

    if (type == ConnectionHandlerType::Server) {
        logger("DEBUG") << "ConnectionHandler [stop()] : Connected_sockets.size() = " << std::to_string(connected_sockets_.size()) << "\n";
        for (auto &connected_socket: connected_sockets_) {
            disconnected(connected_socket);
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

ConnectionHandler::ConnectionHandler(std::string &address, int port, ConnectionHandlerType type,
                                     std::shared_ptr<ThreadPool> thread_pool, bool DEBUG)
    : address(address), port(port), thread_pool_(thread_pool), type(type) {
    logger.init5Levels();
    if (!DEBUG) {
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

    if (!isWork->load()) {
        logger("ERROR") << "ConnectionHandler [createAcceptor()] : isWork = " << isWork->load() << "\n";
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
    return acceptor;
}

std::shared_ptr<tcp::socket> ConnectionHandler::createSocket() {
    return std::make_shared<tcp::socket>(io_context);
}

std::shared_ptr<tcp::socket> ConnectionHandler::accept() {
    if (type == ConnectionHandlerType::Client) {
        logger("ERROR") << "ConnectionHandler [accept()] : ConnectionHandlerType = Client" << "\n";
        return nullptr;
    }

    if (!isWork->load()) {
        logger("ERROR") << "ConnectionHandler [accept()] : isWork = " << isWork->load() << "\n";
        return nullptr;
    }

    auto socket = std::make_shared<tcp::socket>(io_context);
    boost::system::error_code ec;
    acceptor_->accept(*socket, ec);
    acceptor_->non_blocking(true);
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


bool ConnectionHandler::disconnected(ConnectedSocket &connected_socket) {
    boost::system::error_code ec;
    connected_socket.ptr->close(ec);
    if (!ec) {
        logger("INFO") << "ConnectionHandler [disconnected()] : Disconnected socket " << connected_socket.
                getAddressAndPort() << "\n";
        return true;
    } else {
        logger("WARN") << "ConnectionHandler [disconnected()] : Disconnected socket " << connected_socket.
                getAddressAndPort() << "\n";
        return false;
    }
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
        return nullptr;
    } else {
        logger("INFO") << "ConnectionHandler [connect()] : Connect socket " << socket_client_->remote_endpoint().
                address().to_string() << ":" << socket_client_->remote_endpoint().port() << "\n";

        auto self = shared_from_this();
        thread_pool_->enqueue([self]() {
            while (self->isWork->load()) {
                if (!self->is_connected(self->socket_client_)) {
                    self->logger("DEBUG") << "ConnectionHandler [is_connected()] : Socket " << self->address << ":" <<
                            self->port << " is_connected() = false" << "\n";
                    self->stop();
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
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

void ConnectionHandler::listen() {
    if (type == ConnectionHandlerType::Client) {
        logger("ERROR") << "ConnectionHandler [listen()] : ConnectionHandlerType = Client\n";
        return;
    }
    if (!isWork->load()) {
        logger("ERROR") << "ConnectionHandler [listen()] : isWork = false\n";
        return;
    }

    acceptor_->listen();

    auto self = shared_from_this();
    std::shared_ptr<std::atomic<bool> > check_connection = std::make_shared<std::atomic<bool> >(false);
    thread_pool_->enqueue([self,check_connection]() {
        self->logger("INFO") << "ConnectionHandler [listen()] : Start listen connection...\n";
        while (self->isWork->load()) {
            auto socket = self->accept();
            if (socket)
                check_connection->store(true);

            if (self->connected_sockets_.empty())
                check_connection->store(false);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        self->logger("INFO") << "ConnectionHandler [listen()] : Stopped listen connection\n";
    });

    thread_pool_->enqueue([self,check_connection]() {
        while (self->isWork->load()) {
            if (check_connection->load()) {
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
    });
}
