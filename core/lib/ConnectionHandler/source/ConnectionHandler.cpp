#include "ConnectionHandler.hpp"
#include <algorithm>

static std::string dataForLogger(ConnectionHandlerType type) {
    return (type == ConnectionHandlerType::Server ? "[Server] " : "[Client] ");
}

ConnectionHandler::ConnectionHandler(const std::string &address, int port, ConnectionHandlerType type, bool INFO)
    : address(address), port(port), type(type) {
    logger = LoggerFactory::getLogger("ConnectionHandler");
    if (INFO) logger->setLevel(LogLevel::Error);
    else      logger->setLevel(LogLevel::Trace);
}

ConnectionHandler::~ConnectionHandler() { stop(); }

Connection ConnectionHandler::is_connected(ConnectedSocket sock) {
    try {
        sock.ptr->non_blocking(true);
        boost::system::error_code ec;
        char c;
        sock.ptr->receive(boost::asio::buffer(&c, 1),
                                      boost::asio::socket_base::message_peek,
                                      ec);
        sock.ptr->non_blocking(false);
        if (ec == boost::asio::error::would_block) return Connection::Connected;
        if (ec == boost::asio::error::eof) return Connection::Disconnected;
        return Connection::Disconnected;
    } catch (std::exception &e) {
        logger->log(LogLevel::Warn, __func__, dataForLogger(type) + e.what());
        return Connection::Error;
    }

}

ConnectedSocket ConnectionHandler::findConnectedSocket(size_t id) {
    std::lock_guard<std::mutex> lock(connection_data_mutex);
    auto it = std::find_if(connected_sockets_.begin(), connected_sockets_.end(),
        [id](const ConnectedSocket& cs) { return cs.id == id; });
    if (it != connected_sockets_.end()) {
        return *it;  // копия
    }
    return ConnectedSocket();  // пустой
}

std::vector<ConnectedSocket>& ConnectionHandler::getSockets() { return connected_sockets_; }
std::shared_ptr<tcp::acceptor> ConnectionHandler::getAcceptor() { return acceptor_; }
ConnectedSocket ConnectionHandler::getSocket() { return socket_client_; }

void ConnectionHandler::setTaskSocket(std::function<void(ConnectedSocket&)> task) {
    task_ = std::move(task);
}

void ConnectionHandler::setGenerateID(std::function<size_t()> g) {
    generateID = g;
}

void ConnectionHandler::setNewConnectionHandler(std::function<void(ConnectedSocket)> h) {
    newConnection = h;
}

void ConnectionHandler::setCloseConnectionHandler(std::function<void(ConnectedSocket)> h) {
    closeConnection = h;
}

void ConnectionHandler::runTask(ConnectedSocket& connected_socket) {
    try {
        if (task_) task_(connected_socket);
    } catch (std::exception &e) {
        logger->log(LogLevel::Error, "task", dataForLogger(type) + e.what());
    }
}

bool ConnectionHandler::getIsWork() const { return isWork.load(); }

void ConnectionHandler::runAcceptorThread() {
    auto self = shared_from_this();
    if (type == ConnectionHandlerType::Server)
        acceptor_thread = std::make_shared<std::thread>([self]() {
            self->logger->log(LogLevel::Info, "runAcceptorThread", dataForLogger(self->type) + "Start accept connection");
            while (self->isWork.load()) {
                ConnectedSocket cs = self->accept(false);
                if (cs.ptr) {
                    self->runTask(cs);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            self->logger->log(LogLevel::Info, "runAcceptorThread", dataForLogger(self->type) + "Stop accept connection");
        });
}

void ConnectionHandler::runConnectionCheckThread() {
    auto self = shared_from_this();
    connection_thread = std::make_shared<std::thread>([self]() {
        try {
            self->logger->log(LogLevel::Debug, "ConnectionCheckThread",
                              dataForLogger(self->type) + "Start check connection...");
            auto start = std::chrono::steady_clock::now();
            while (self->isWork.load()) {
                if (self->type == ConnectionHandlerType::Server) {
                    if (std::chrono::steady_clock::now() - start > std::chrono::seconds(1)) {
                        start = std::chrono::steady_clock::now();
                        self->logger->log(LogLevel::Debug, "connection_thread", dataForLogger(self->type)  +  "self->connected_sockets_.size() = " + std::to_string(self->connected_sockets_.size()));
                    }
                    for (auto socket: self->connected_sockets_) {
                        if (self->is_connected(socket) != Connection::Connected) {
                            self->logger->log(LogLevel::Debug, "ConnectionCheckThread", dataForLogger(self->type) +
                                                  "Socket " + socket.getAddressAndPort() +
                                                  " is_connected() = Disconnected");

                            self->disconnected(socket, true);
                            break;
                        }
                    }
                } else if (self->type == ConnectionHandlerType::Client) {
                    if (self->is_connected(self->socket_client_) != Connection::Connected) {
                        self->logger->log(LogLevel::Debug, "ConnectionCheckThread", dataForLogger(self->type) +
                                              "Socket " + self->address + ":" + std::to_string(self->port) +
                                              " is_connected() = false");
                        self->logger->log(LogLevel::Info, "ConnectionCheckThread", dataForLogger(self->type) +
                                                                               "Connection lost, closing socket");
                        if (self->socket_client_.ptr && self->socket_client_.ptr->is_open()) {
                            boost::system::error_code ec;
                            self->socket_client_.ptr->close(ec);
                            if (ec) {
                                self->logger->log(LogLevel::Warn, "ConnectionCheckThread",
                                                  dataForLogger(self->type) + "Close error: " + ec.message());
                            }
                        }
                        break;
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            self->logger->log(LogLevel::Debug, "ConnectionCheckThread",dataForLogger(self->type) + "Stop check connection");
        } catch (std::exception &e) {
            self->logger->log(LogLevel::Error, "ConnectionCheckThread", dataForLogger(self->type)  + e.what());

        }
    });
}

void ConnectionHandler::start() {
    if (!isWork.load()) {
        isWork.store(true);
        logger->log(LogLevel::Info, __func__, dataForLogger(type) + "Start");
        if (type == ConnectionHandlerType::Server)
            acceptor_ = createAcceptor();
    } else {
        stop();
    }
}

void ConnectionHandler::stop() {
    if (!isWork.load()) {return;}
    isWork.store(false);
    logger->log(LogLevel::Debug, __func__, dataForLogger(type) + "Stopping...");

    if (acceptor_thread && acceptor_thread->joinable()) {
        acceptor_thread->join();
        acceptor_thread.reset();
    }
    if (connection_thread && connection_thread->joinable()) {
        connection_thread->join();
        connection_thread.reset();
    }
    if (acceptor_ && acceptor_->is_open()) {
        acceptor_->cancel();
        acceptor_->close();
        logger->log(LogLevel::Debug, __func__, dataForLogger(type) + "Acceptor closed");
        acceptor_.reset();
    } else if (acceptor_) {
        acceptor_.reset();
    }

    if (type == ConnectionHandlerType::Server) {
        std::vector<ConnectedSocket> to_disconnect;
        {
            std::lock_guard<std::mutex> lock(connection_data_mutex);
            to_disconnect.swap(connected_sockets_);
        }
        for (auto &sock : to_disconnect)
            disconnected(sock, true);
    }

    if (socket_client_.ptr && socket_client_.ptr->is_open()) {
        connection_data_mutex.lock();
        disconnect();
        socket_client_.ptr.reset();
        connection_data_mutex.unlock();
    }
    if (socket_client_.ptr)
        socket_client_.ptr.reset();

    logger->log(LogLevel::Info, __func__, dataForLogger(type) + "Stop");
}

std::shared_ptr<tcp::acceptor> ConnectionHandler::createAcceptor() {
    logger->log(LogLevel::Debug, __func__, dataForLogger(type) + "Creating acceptor...");
    if (type == ConnectionHandlerType::Client) {
        logger->log(LogLevel::Error, __func__, dataForLogger(type) + "ConnectionHandlerType = Client");
        return nullptr;
    }
    if (!isWork.load()) {
        logger->log(LogLevel::Error, __func__, dataForLogger(type) + "isWork = false");
        return nullptr;
    }
    auto acceptor = std::make_shared<tcp::acceptor>(io_context);
    acceptor->open(tcp::v4());
    boost::system::error_code ec;
    acceptor->set_option(tcp::acceptor::reuse_address(true));
    acceptor->bind(tcp::endpoint(boost::asio::ip::address::from_string(address), port), ec);
    if (!ec || ec == boost::asio::error::address_in_use) {
        if (ec == boost::asio::error::address_in_use)
            logger->log(LogLevel::Warn, __func__, dataForLogger(type) + "Address_in_use");
        logger->log(LogLevel::Debug, __func__, dataForLogger(type) + "Create acceptor successful");
        logger->log(LogLevel::Info, __func__, dataForLogger(type) + "Acceptor on " + address + ":" + std::to_string(port));
    } else {
        logger->log(LogLevel::Error, __func__, dataForLogger(type) + "Create acceptor fail: " + ec.message());
        stop();
        return nullptr;
    }
    return acceptor;
}

std::shared_ptr<tcp::socket> ConnectionHandler::createSocket() { return std::make_shared<tcp::socket>(io_context); }

ConnectedSocket ConnectionHandler::accept(bool blocking) {
    if (type == ConnectionHandlerType::Client) {
        logger->log(LogLevel::Error, __func__, dataForLogger(type) + "ConnectionHandlerType = Client");
        return ConnectedSocket();
    }
    if (!isWork.load()) {
        logger->log(LogLevel::Error, __func__, dataForLogger(type) + "isWork = false");
        return ConnectedSocket();
    }
    auto socket = createSocket();
    boost::system::error_code ec;
    acceptor_->non_blocking(!blocking);
    acceptor_->accept(*socket, ec);
    if (ec == boost::asio::error::would_block)
        return ConnectedSocket();
    if (ec) {
        logger->log(LogLevel::Error, __func__, dataForLogger(type) + ec.message());
        return ConnectedSocket();
    }
    if (!generateID) {
        IdConnectionGenerator generator;
        generateID = [&generator](){return generator.generate();};
    }
    ConnectedSocket connected_socket(socket, generateID());
    if (newConnection) newConnection(connected_socket);
    logger->log(LogLevel::Info, __func__, dataForLogger(type) + "Socket " + connected_socket.getAddressAndPort() + " id: " + std::to_string(connected_socket.id));
    std::lock_guard<std::mutex> lock(connection_data_mutex);
    connected_sockets_.push_back(connected_socket);
    // Возвращаем копию
    return connected_socket;
}

bool ConnectionHandler::disconnected(ConnectedSocket &connected_socket, bool delete_socket) {
    boost::system::error_code ec;
    try {
        connected_socket.ptr->close(ec);
        if (closeConnection) closeConnection(connected_socket);
        if (!ec) {
            logger->log(LogLevel::Info, __func__, dataForLogger(type) + "Disconnected " + connected_socket.getAddressAndPort());
        } else {
            logger->log(LogLevel::Warn, __func__, dataForLogger(type) + "Close error: " + ec.message());
        }
        if (delete_socket) {
            std::lock_guard<std::mutex> lock(connection_data_mutex);
            auto it = std::find_if(connected_sockets_.begin(), connected_sockets_.end(),
                                   [&](const ConnectedSocket &cs) { return cs.ptr == connected_socket.ptr; });
            if (it != connected_sockets_.end()) {
                connected_sockets_.erase(it);
                logger->log(LogLevel::Debug, __func__, dataForLogger(type) + "Removed from connected_sockets");
            }
        }
        return !ec;
    } catch (std::exception &e) {
        logger->log(LogLevel::Error, __func__, dataForLogger(type) + e.what());
        return false;
    }
}

void ConnectionHandler::disconnect(bool needJoinThread) {
    if (type == ConnectionHandlerType::Server) {
        logger->log(LogLevel::Error, __func__, dataForLogger(type) + "Called on Server");
        return;
    }
    if (!socket_client_.ptr || !socket_client_.ptr->is_open()) {
        logger->log(LogLevel::Debug, __func__, dataForLogger(type) + "Socket already closed");
        return;
    }
    isWork.store(false);
    boost::system::error_code ec;
    if (connection_thread && connection_thread->joinable() && needJoinThread) {
        connection_thread->join();
        connection_thread.reset();
    }
    if (socket_client_.ptr->is_open()) {
        socket_client_.ptr->close(ec);
        if (closeConnection) closeConnection(socket_client_);
        if (ec) {
            logger->log(LogLevel::Error, __func__, dataForLogger(type) + "Close error: " + ec.message());
        } else {
            logger->log(LogLevel::Info, __func__, dataForLogger(type) + "Disconnect " + address + ":" + std::to_string(port));
        }
    }
}

ConnectedSocket ConnectionHandler::connect(int numTry) {
    if (type == ConnectionHandlerType::Server) {
        logger->log(LogLevel::Error, __func__, dataForLogger(type) + "ConnectionHandlerType = Server");
        return ConnectedSocket();
    }
    boost::system::error_code ec;
    for (int i = 1; i <= numTry; ++i) {
        logger->log(LogLevel::Debug, __func__, dataForLogger(type) + "Try " + std::to_string(i) + "\tconnection");
        auto socket = createSocket();
        socket_client_.ptr = socket;
        socket_client_.ptr->connect(tcp::endpoint(boost::asio::ip::address::from_string(address), port), ec);
        if (ec) {
            logger->log(LogLevel::Warn, __func__, dataForLogger(type) + "Connect socket " + address + ":" + std::to_string(port));
        } else {
            logger->log(LogLevel::Info, __func__, dataForLogger(type) + "Connect socket " +
                        socket_client_.ptr->remote_endpoint().address().to_string() + ":" +
                        std::to_string(socket_client_.ptr->remote_endpoint().port()));
            // Устанавливаем id для клиентского сокета
            if (!generateID) {
                IdConnectionGenerator generator;
                generateID = [&generator](){return generator.generate();};
            }
            socket_client_.id = generateID();
            runConnectionCheckThread();
            runTask(socket_client_);
            if (newConnection) newConnection(socket_client_);
            return socket_client_;  // возвращаем копию
        }
        if (i == numTry) stop();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return ConnectedSocket();
}

void ConnectionHandler::listen() {
    if (type == ConnectionHandlerType::Client) {
        logger->log(LogLevel::Error, __func__, dataForLogger(type) + "ConnectionHandlerType = Client");
        return;
    }
    if (!isWork.load()) {
        logger->log(LogLevel::Error, __func__, dataForLogger(type) + "isWork = false");
        return;
    }
    acceptor_->listen();
    runAcceptorThread();
    runConnectionCheckThread();
}