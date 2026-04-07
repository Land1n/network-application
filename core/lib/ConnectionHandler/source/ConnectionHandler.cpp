//
// Created by ivan on 07.03.2026.
//
#include "ConnectionHandler.hpp"
#include <algorithm>

#include "sdrlogger/sdrlogger.h"

std::string dataForLogger(std::string name_function, ConnectionHandlerType type) {
    return std::string("\tConnectionHandler { ") + (type == ConnectionHandlerType::Server ? "Server" : "Client") +
           " } [" + name_function + "()] :\t";
}

Connection ConnectionHandler::is_connected(std::shared_ptr<tcp::socket> sock) {
    try {
        sock->non_blocking(true);
        boost::system::error_code ec;
        char c;
        size_t bytes = sock->read_some(boost::asio::buffer(&c, 1), ec);
        sock->non_blocking(false);

        if (ec == boost::asio::error::would_block) {
            return Connection::Connected;
        }
        if (ec == boost::asio::error::eof || bytes == 0) {
            return Connection::Disconnected;
        }
        if (!ec) {
            return Connection::Disconnected;
        }
        return Connection::Disconnected;
    } catch (std::exception &e) {
        logger_mutex->lock();
        logger("WARN") << dataForLogger(__func__, type) << e.what() << "\n";
        logger_mutex->unlock();
        return Connection::Error;
    }
}

std::vector<ConnectedSocket> &ConnectionHandler::getSockets() {
    // while(!connection_data_mutex.try_lock())
    return connected_sockets_;
}

std::shared_ptr<tcp::acceptor> ConnectionHandler::getAcceptor() {
    return acceptor_;
}

std::shared_ptr<tcp::socket> ConnectionHandler::getSocket() {
    return socket_client_;
};

void ConnectionHandler::setLoggerMutex(std::shared_ptr<std::mutex> logger_mutex) {
    this->logger_mutex = logger_mutex;
}

void ConnectionHandler::setTaskSocket(std::function<void(std::shared_ptr<tcp::socket>)> task) {
    task_ = std::move(task);
}

bool ConnectionHandler::isInWork() const {
    return isWork.load();
}

void ConnectionHandler::start() {
    if (!isWork.load()) {
        isWork.store(true);
        logger("INFO") << dataForLogger(__func__, type) << "Start\n";
        setLoggerMutex();

        if (type == ConnectionHandlerType::Server)
            acceptor_ = createAcceptor();
    } else {
        stop();
    }
}

void ConnectionHandler::stop() {
    if (!isWork.load()) return;
    isWork.store(false);

    logger("DEBUG") << dataForLogger(__func__, type) << "Stopping...\n";
    if (acceptor_thread != nullptr) {
        acceptor_thread->join();
        acceptor_thread.reset();
    }

    if (connection_thread != nullptr) {
        connection_thread->join();
        connection_thread.reset();
    }

    if (acceptor_ != nullptr && acceptor_->is_open()) {
        acceptor_->cancel();
        acceptor_->close();
        logger("DEBUG") << dataForLogger(__func__, type) << "Acceptor closed\n";
        acceptor_.reset();
    } else if (acceptor_ != nullptr) {
        acceptor_.reset();
    }

    if (type == ConnectionHandlerType::Server) {
        std::vector<ConnectedSocket> to_disconnect;
        {
            std::lock_guard<std::mutex> lock(connection_data_mutex);
            to_disconnect.swap(connected_sockets_);
        }
        for (auto &sock: to_disconnect) {
            disconnected(sock, true); // больше не блокирует мьютекс
        }
    }

    if (socket_client_ != nullptr && socket_client_->is_open()) {
        connection_data_mutex.lock();
        disconnect();
        socket_client_.reset();
        connection_data_mutex.unlock();
    }
    if (socket_client_ != nullptr)
        socket_client_.reset();


    logger("INFO") << dataForLogger(__func__, type) << "Stop\n";
}

ConnectionHandler::ConnectionHandler(std::string &address, int port, ConnectionHandlerType type, bool INFO)
    : address(address), port(port), type(type) {
    logger.init5Levels();
    if (INFO) {
        logger.setLogLevel("ERROR");
    }
}

ConnectionHandler::~ConnectionHandler() {
    stop();
}

std::shared_ptr<tcp::acceptor> ConnectionHandler::createAcceptor() {
    logger_mutex->lock();
    logger("DEBUG") << dataForLogger(__func__, type) << "Creating acceptor..." << "\n";
    logger_mutex->unlock();
    if (type == ConnectionHandlerType::Client) {
        logger_mutex->lock();
        logger("ERROR") << dataForLogger(__func__, type) << "ConnectionHandlerType = Client" << "\n";
        logger_mutex->unlock();
        return nullptr;
    }

    if (!isWork.load()) {
        logger_mutex->lock();
        logger("ERROR") << dataForLogger(__func__, type) << "isWork = " << isWork.load() << "\n";
        logger_mutex->unlock();
        return nullptr;
    }
    std::shared_ptr<tcp::acceptor> acceptor = std::make_shared<tcp::acceptor>(io_context);
    acceptor->open(tcp::v4());
    boost::system::error_code error_code;
    acceptor->set_option(tcp::acceptor::reuse_address(true));
    acceptor->bind(tcp::endpoint(boost::asio::ip::address::from_string(address), port), error_code);
    if (!error_code || error_code == boost::asio::error::address_in_use) {
        if (error_code == boost::asio::error::address_in_use)
            logger("WARN") << dataForLogger(__func__, type) << "Address_in_use" << "\n";
        logger("DEBUG") << dataForLogger(__func__, type) << "Create acceptor successful" << "\n";
        logger("INFO") << dataForLogger(__func__, type) << "Acceptor on " << address << ":" << port << "\n";
    } else {
        logger_mutex->lock();
        logger("ERROR") << dataForLogger(__func__, type) << "Create acceptor fail" << "\n";
        logger("ERROR") << dataForLogger(__func__, type) << error_code.what() << "\n";
        logger_mutex->unlock();
        stop();
        return nullptr;
    }
    if (acceptor == nullptr) {
        logger_mutex->lock();
        logger("WARN") << dataForLogger(__func__, type) << "Acceptor=nullptr\n";
        logger_mutex->unlock();
    }
    return acceptor;
}

std::shared_ptr<tcp::socket> ConnectionHandler::createSocket() {
    return std::make_shared<tcp::socket>(io_context);
}

std::shared_ptr<tcp::socket> ConnectionHandler::accept(bool blocking) {
    if (type == ConnectionHandlerType::Client) {
        logger_mutex->lock();
        logger("ERROR") << dataForLogger(__func__, type) << "ConnectionHandlerType = Client" << "\n";
        logger_mutex->unlock();
        return nullptr;
    }

    if (!isWork.load()) {
        logger_mutex->lock();
        logger("ERROR") << dataForLogger(__func__, type) << "isWork = " << isWork.load() << "\n";
        logger_mutex->unlock();
        return nullptr;
    }

    auto socket = std::make_shared<tcp::socket>(io_context);
    boost::system::error_code ec;
    acceptor_->non_blocking(!blocking);
    acceptor_->accept(*socket, ec);
    if (ec == boost::asio::error::would_block)
        return nullptr;
    else if (ec) {
        logger_mutex->lock();
        logger("ERROR") << dataForLogger(__func__, type) << ec.message() << "\n";
        logger_mutex->unlock();
        return nullptr;
    }
    auto connected_socket = ConnectedSocket(socket);
    logger_mutex->lock();
    logger("INFO") << dataForLogger(__func__, type) << "Socket " << connected_socket.getAddressAndPort() << "\n";
    logger_mutex->unlock();
    connection_data_mutex.lock();
    connected_sockets_.push_back(connected_socket);
    connection_data_mutex.unlock();

    return socket;
}

bool ConnectionHandler::disconnected(ConnectedSocket &connected_socket, bool delete_socket) {
    boost::system::error_code ec;
    try {
        connected_socket.ptr->close(ec);
        if (!ec) {
            logger("INFO") << dataForLogger(__func__, type) << "Disconnected " << connected_socket.getAddressAndPort()
                    << "\n";
        } else {
            logger("WARN") << dataForLogger(__func__, type) << "Close error: " << ec.message() << "\n";
        }

        if (delete_socket) {
            std::lock_guard<std::mutex> lock(connection_data_mutex);
            auto it = std::find_if(connected_sockets_.begin(), connected_sockets_.end(),
                                   [&](const ConnectedSocket &cs) { return cs.ptr == connected_socket.ptr; });
            if (it != connected_sockets_.end()) {
                connected_sockets_.erase(it);
                logger("DEBUG") << dataForLogger(__func__, type) << "Removed from connected_sockets\n";
            }
        }
        return !ec;
    } catch (std::exception &e) {
        logger("ERROR") << dataForLogger(__func__, type) << e.what() << "\n";
        return false;
    }
}

void ConnectionHandler::disconnect() {
    if (type == ConnectionHandlerType::Server) {
        logger("ERROR") << dataForLogger(__func__, type) << "Called on Server\n";
        return;
    }
    if (!socket_client_ || !socket_client_->is_open()) {
        logger("DEBUG") << "Socket already closed\n";
        return;
    }
    isWork.store(false);
    boost::system::error_code ec;
    if (connection_thread && connection_thread->joinable()) {
        connection_thread->join();
        connection_thread.reset();
    }

    if (socket_client_->is_open())
        socket_client_->close(ec);
    if (ec) {
        logger("ERROR") << dataForLogger(__func__, type) << "Close error: " << ec.message() << "\n";
    } else {
        logger("INFO") << dataForLogger(__func__, type) << "Disconnected " << address << ":" << port << "\n";
    }
}

std::shared_ptr<tcp::socket> ConnectionHandler::connect(int numTry) {
    if (type == ConnectionHandlerType::Server) {
        logger_mutex->lock();
        logger("ERROR") << dataForLogger(__func__, type) << "ConnectionHandlerType = Server" << "\n";
        logger_mutex->unlock();
        return nullptr;
    }
    boost::system::error_code ec;
    for (int i = 1; i <= numTry; i++) {
        logger_mutex->lock();
        logger("DEBUG") << dataForLogger(__func__, type) << "Try " << i << "\tconnection\n";
        logger_mutex->unlock();
        socket_client_ = createSocket();
        socket_client_->connect(tcp::endpoint(boost::asio::ip::address::from_string(address), port), ec);
        if (ec) {
            logger_mutex->lock();
            logger("WARN") << dataForLogger(__func__, type) << "Connect socket " << address << ":" << port << "\n";
            logger_mutex->unlock();
        } else {
            logger_mutex->lock();
            logger("INFO") << dataForLogger(__func__, type) << "Connect socket " << socket_client_->remote_endpoint().
                    address().to_string() << ":" << socket_client_->remote_endpoint().port() << "\n";
            logger_mutex->unlock();
            auto self = shared_from_this();
            connection_thread = std::make_shared<std::thread>([self]() {
                try {
                    self->logger_mutex->lock();
                    self->logger("DEBUG") << dataForLogger("connection_thread", self->type) <<
                            "Start check connection...\n";
                    self->logger_mutex->unlock();
                    while (self->isWork.load()) {
                        if (self->is_connected(self->socket_client_) != Connection::Connected) {
                            self->logger_mutex->lock();
                            self->logger("DEBUG") << dataForLogger("connection_thread", self->type)
                                    << "Socket " << self->address << ":" << self->port
                                    << " is_connected() = false" << "\n";

                            self->logger_mutex->unlock();
                            // boost::system::error_code ec;
                            // self->logger("INFO") << dataForLogger("connection_thread", self->type)<< "Connection lost, closing socket\n";
                            // if (self->socket_client_ && self->socket_client_->is_open()) {
                            //     self->socket_client_->close(ec);
                            //     if (ec) {
                            //         self->logger("WARN") << dataForLogger("connection_thread", self->type) << "Close error: " << ec.message() << "\n";
                            //     }
                            // }
                            //
                            // // Сигнализируем, что работа завершена
                            // self->isWork.store(false);
                            break;
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                    self->logger_mutex->lock();
                    self->logger("DEBUG") << dataForLogger("connection_thread", self->type) << "Stop check connection\n";
                    self->logger_mutex->unlock();
                } catch (...) {
                    self->logger("ERROR") << dataForLogger("connection_thread", self->type) << "Unknown fatal error in connection_thread\n";
                }
            });
            if (task_) {
                std::thread task_thread([self]() {
                    self->task_(self->socket_client_);
                });
                task_thread.detach();
            }
            return socket_client_;
        }
        if (i == numTry)
            stop();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return nullptr;
}

void ConnectionHandler::listen() {
    if (type == ConnectionHandlerType::Client) {
        logger_mutex->lock();
        logger("ERROR") << dataForLogger(__func__, type) << "ConnectionHandlerType = Client\n";
        logger_mutex->unlock();
        return;
    }
    if (!isWork.load()) {
        logger_mutex->lock();
        logger("ERROR") << dataForLogger(__func__, type) << "isWork = false\n";
        logger_mutex->unlock();
        return;
    }
    boost::system::error_code ec;
    acceptor_->listen();
    auto self = shared_from_this();
    acceptor_thread = std::make_shared<std::thread>([self]() {
        while (self->isWork.load()) {
            auto socket = self->accept(false);
            if (socket) {
                if (self->task_) {
                    std::thread task_thread([socket,self]() {
                        self->task_(socket);
                    });
                    task_thread.detach();
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        self->logger_mutex->lock();
        self->logger("INFO") << dataForLogger("acceptor_thread", self->type) << "Stopped listen connection\n";
        self->logger_mutex->unlock();
    });

    connection_thread = std::make_shared<std::thread>([self]() {
        try {
            auto start = std::chrono::steady_clock::now();

            self->logger_mutex->lock();
            self->logger("DEBUG") << dataForLogger("connection_thread", self->type) << "Start check connection\n";
            self->logger_mutex->unlock();
            while (self->isWork.load()) {
                if (std::chrono::steady_clock::now() - start > std::chrono::seconds(2)) {
                    start = std::chrono::steady_clock::now();
                    self->logger_mutex->lock();
                    self->logger("DEBUG") << dataForLogger("connection_thread", self->type)
                            << "self->connected_sockets_.size() = " << std::to_string(self->connected_sockets_.size())
                            <<
                            "\n";
                    self->logger_mutex->unlock();
                }
                if (!self->connected_sockets_.empty()) {
                    for (int i = 0; i < self->connected_sockets_.size(); i++)
                        if (self->is_connected(self->connected_sockets_[i].ptr) == Connection::Disconnected ||
                            self->is_connected(self->connected_sockets_[i].ptr) == Connection::Error) {
                            ConnectedSocket cs(self->connected_sockets_[i].ptr);
                            self->logger_mutex->lock();
                            self->logger("DEBUG") << dataForLogger("connection_thread", self->type) << "Socket " << cs.
                                    getAddressAndPort() << " is_connected() = Disconnected" << "\n";
                            self->logger_mutex->unlock();
                            self->disconnected(cs, true);
                            break;
                        }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            self->logger_mutex->lock();
            self->logger("DEBUG") << dataForLogger("connection_thread", self->type) << "Stop check connection\n";
            self->logger_mutex->unlock();
        } catch (std::exception &e) {
            self->logger_mutex->lock();
            self->logger("ERROR") << dataForLogger("connection_thread", self->type) << e.what() << "\n";
            self->logger_mutex->unlock();
        }
    });
}
