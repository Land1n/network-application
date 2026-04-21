#include "ConnectionHandler.hpp"
#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>

static std::string dataForLogger(ConnectionHandlerType type) {
    return (type == ConnectionHandlerType::Server ? "[Server] " : "[Client] ");
}

ConnectionHandler::ConnectionHandler(const std::string &address, int port, ConnectionHandlerType type, bool INFO)
    : address(address), port(port), type(type),
      acceptorWorker(INFO), connectionWorker(INFO), taskWorker(INFO) {
    if (INFO) logger.setLevel(LogLevel::Error);
    else logger.setLevel(LogLevel::Trace);

    generateID = [this]() { return defaultGenerator.generate(); };
}

ConnectionHandler::~ConnectionHandler() { stop(); }

Connection ConnectionHandler::is_connected(ConnectedSocket sock) {
    if (!sock.ptr || !sock.ptr->is_open())
        return Connection::Disconnected;

    try {
        sock.ptr->non_blocking(true);
        boost::system::error_code ec;
        char c;
        sock.ptr->receive(boost::asio::buffer(&c, 1),
                          boost::asio::socket_base::message_peek, ec);
        sock.ptr->non_blocking(false);
        if (ec == boost::asio::error::would_block) return Connection::Connected;
        if (ec == boost::asio::error::eof) return Connection::Disconnected;
        return Connection::Disconnected;
    } catch (std::exception &e) {
        logger.log(LogLevel::Warn, __func__, dataForLogger(type) + e.what());
        return Connection::Error;
    }
}

ConnectedSocket ConnectionHandler::findConnectedSocket(size_t id) {
    std::lock_guard<std::mutex> lock(connection_data_mutex);
    auto it = std::find_if(connected_sockets_.begin(), connected_sockets_.end(),
                           [id](const ConnectedSocket &cs) { return cs.id == id; });
    if (it != connected_sockets_.end()) return *it;
    return ConnectedSocket();
}

std::vector<ConnectedSocket>& ConnectionHandler::getSockets() { return connected_sockets_; }
std::shared_ptr<tcp::acceptor> ConnectionHandler::getAcceptor() { return acceptor_; }
ConnectedSocket& ConnectionHandler::getSocket() { return socket_client_; }
bool ConnectionHandler::getIsWork() const { return isWork.load(); }

void ConnectionHandler::setTaskSocket(std::function<void(ConnectedSocket&)> task) { task_ = std::move(task); }
void ConnectionHandler::setGenerateID(std::function<size_t()> g) { generateID = std::move(g); }
void ConnectionHandler::setNewConnectionHandler(std::function<void(ConnectedSocket)> h) { newConnection = std::move(h); }
void ConnectionHandler::setCloseConnectionHandler(std::function<void(ConnectedSocket)> h) { closeConnection = std::move(h); }

void ConnectionHandler::start() {
    if (!isWork.load()) {
        isWork.store(true);
        logger.log(LogLevel::Info, __func__, dataForLogger(type) + "Start");
        if (type == ConnectionHandlerType::Server) {
            acceptor_ = createAcceptor();
            if (acceptor_) acceptorWorker.start();
            connectionWorker.start();
            taskWorker.start();
        }
    } else {
        stop();
    }
}

void ConnectionHandler::stop() {
    if (!isWork.load()) return;
    isWork.store(false);
    logger.log(LogLevel::Debug, __func__, dataForLogger(type) + "Stopping...");

    acceptorWorker.stop(true);
    connectionWorker.stop(true);
    taskWorker.stop(true);

    if (acceptor_ && acceptor_->is_open()) {
        boost::system::error_code ec;
        acceptor_->cancel(ec);
        acceptor_->close(ec);
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

    // Закрываем клиентский сокет (воркеры уже остановлены)
    if (socket_client_.ptr && socket_client_.ptr->is_open()) {
        boost::system::error_code ec;
        socket_client_.ptr->close(ec);
        if (closeConnection) closeConnection(socket_client_);
        socket_client_.ptr.reset();
    }

    logger.log(LogLevel::Info, __func__, dataForLogger(type) + "Stop");
}

std::shared_ptr<tcp::acceptor> ConnectionHandler::createAcceptor() {
    if (type == ConnectionHandlerType::Client) {
        logger.log(LogLevel::Error, __func__, dataForLogger(type) + "Called on Client");
        return nullptr;
    }
    if (!isWork.load()) {
        logger.log(LogLevel::Error, __func__, dataForLogger(type) + "isWork = false");
        return nullptr;
    }
    auto acceptor = std::make_shared<tcp::acceptor>(io_context);
    acceptor->open(tcp::v4());
    boost::system::error_code ec;
    acceptor->set_option(tcp::acceptor::reuse_address(true));
    acceptor->bind(tcp::endpoint(boost::asio::ip::address::from_string(address), port), ec);
    if (ec && ec != boost::asio::error::address_in_use) {
        logger.log(LogLevel::Error, __func__, dataForLogger(type) + "Bind error: " + ec.message());
        stop();
        return nullptr;
    }
    if (ec == boost::asio::error::address_in_use)
        logger.log(LogLevel::Warn, __func__, dataForLogger(type) + "Address already in use");
    logger.log(LogLevel::Info, __func__, dataForLogger(type) + "Acceptor on " + address + ":" + std::to_string(port));
    return acceptor;
}

std::shared_ptr<tcp::socket> ConnectionHandler::createSocket() {
    return std::make_shared<tcp::socket>(io_context);
}

ConnectedSocket ConnectionHandler::accept(bool blocking) {
    if (type == ConnectionHandlerType::Client) {
        logger.log(LogLevel::Error, __func__, dataForLogger(type) + "Called on Client");
        return ConnectedSocket();
    }
    if (!isWork.load() || !acceptor_) {
        logger.log(LogLevel::Error, __func__, dataForLogger(type) + "Not ready");
        return ConnectedSocket();
    }
    auto socket = createSocket();
    boost::system::error_code ec;
    acceptor_->non_blocking(!blocking);
    acceptor_->accept(*socket, ec);
    if (ec == boost::asio::error::would_block)
        return ConnectedSocket();
    if (ec) {
        logger.log(LogLevel::Error, __func__, dataForLogger(type) + ec.message());
        return ConnectedSocket();
    }

    ConnectedSocket connected_socket(socket, generateID());
    if (newConnection) newConnection(connected_socket);
    logger.log(LogLevel::Info, __func__, dataForLogger(type) + "Socket " + connected_socket.getAddressAndPort() +
               " id: " + std::to_string(connected_socket.id));

    std::lock_guard<std::mutex> lock(connection_data_mutex);
    connected_sockets_.push_back(connected_socket);
    return connected_socket;
}

bool ConnectionHandler::disconnected(ConnectedSocket &connected_socket, bool delete_socket) {
    boost::system::error_code ec;
    try {
        if (connected_socket.ptr && connected_socket.ptr->is_open())
            connected_socket.ptr->close(ec);
        if (closeConnection) closeConnection(connected_socket);
        if (!ec) {
            logger.log(LogLevel::Info, __func__, dataForLogger(type) + "Disconnected " + connected_socket.getAddressAndPort());
        } else {
            logger.log(LogLevel::Warn, __func__, dataForLogger(type) + "Close error: " + ec.message());
        }
        if (delete_socket) {
            std::lock_guard<std::mutex> lock(connection_data_mutex);
            auto it = std::find_if(connected_sockets_.begin(), connected_sockets_.end(),
                                   [&](const ConnectedSocket &cs) { return cs.ptr == connected_socket.ptr; });
            if (it != connected_sockets_.end()) {
                connected_sockets_.erase(it);
                logger.log(LogLevel::Debug, __func__, dataForLogger(type) + "Removed from connected_sockets");
            }
        }
        return !ec;
    } catch (std::exception &e) {
        logger.log(LogLevel::Error, __func__, dataForLogger(type) + e.what());
        return false;
    }
}

void ConnectionHandler::disconnect(bool needJoinThread) {
    if (type == ConnectionHandlerType::Server) {
        logger.log(LogLevel::Error, __func__, dataForLogger(type) + "Called on Server");
        return;
    }
    if (!socket_client_.ptr || !socket_client_.ptr->is_open()) {
        logger.log(LogLevel::Debug, __func__, dataForLogger(type) + "Socket already closed");
        return;
    }
    isWork.store(false);
    if (needJoinThread)
        connectionWorker.stop(true);
    boost::system::error_code ec;
    socket_client_.ptr->close(ec);
    if (closeConnection) closeConnection(socket_client_);
    if (ec)
        logger.log(LogLevel::Error, __func__, dataForLogger(type) + "Close error: " + ec.message());
    else
        logger.log(LogLevel::Info, __func__, dataForLogger(type) + "Disconnect " + address + ":" + std::to_string(port));
}

ConnectedSocket ConnectionHandler::connect(int numTry) {
    if (type == ConnectionHandlerType::Server) {
        logger.log(LogLevel::Error, __func__, dataForLogger(type) + "Called on Server");
        return ConnectedSocket();
    }
    boost::system::error_code ec;
    for (int i = 1; i <= numTry; ++i) {
        logger.log(LogLevel::Debug, __func__, dataForLogger(type) + "Try " + std::to_string(i));
        auto socket = createSocket();
        socket_client_.ptr = socket;
        socket_client_.ptr->connect(tcp::endpoint(boost::asio::ip::address::from_string(address), port), ec);
        if (!ec) {
            logger.log(LogLevel::Info, __func__, dataForLogger(type) + "Connected to " +
                       socket_client_.ptr->remote_endpoint().address().to_string() + ":" +
                       std::to_string(socket_client_.ptr->remote_endpoint().port()));
            socket_client_.id = generateID();
            connectionWorker.start();
            connectionWorker.addTask([this]() { runConnectionCheckTask(); });
            if (task_) {
                taskWorker.addTask([this, sock = socket_client_]() mutable { task_(sock); });
            }
            if (newConnection) newConnection(socket_client_);
            return socket_client_;
        }
        logger.log(LogLevel::Warn, __func__, dataForLogger(type) + "Connect failed: " + ec.message());
        if (i == numTry) stop();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return ConnectedSocket();
}

void ConnectionHandler::listen() {
    if (type == ConnectionHandlerType::Client) {
        logger.log(LogLevel::Error, __func__, dataForLogger(type) + "Called on Client");
        return;
    }
    if (!isWork.load() || !acceptor_) {
        logger.log(LogLevel::Error, __func__, dataForLogger(type) + "Not ready");
        return;
    }
    acceptor_->listen();
    acceptorWorker.addTask([this]() { runAcceptorTask(); });
    connectionWorker.addTask([this]() { runConnectionCheckTask(); });
}

void ConnectionHandler::runAcceptorTask() {
    logger.log(LogLevel::Info, __func__, dataForLogger(type) + "Acceptor task started");
    while (isWork.load()) {
        ConnectedSocket cs = accept(false);
        if (cs.ptr) {
            if (task_) {
                taskWorker.addTask([this, cs]() mutable { task_(cs); });
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    logger.log(LogLevel::Info, __func__, dataForLogger(type) + "Acceptor task finished");
}

void ConnectionHandler::runConnectionCheckTask() {
    logger.log(LogLevel::Debug, __func__, dataForLogger(type) + "Connection check task started");
    auto start = std::chrono::steady_clock::now();
    while (isWork.load()) {
        if (type == ConnectionHandlerType::Server) {
            if (std::chrono::steady_clock::now() - start > std::chrono::seconds(5)) {
                start = std::chrono::steady_clock::now();
                logger.log(LogLevel::Debug, "connection_check",
                           dataForLogger(type) + "connected_sockets_.size() = " + std::to_string(connected_sockets_.size()));
            }
            std::vector<ConnectedSocket> socks_copy;
            {
                std::lock_guard<std::mutex> lock(connection_data_mutex);
                socks_copy = connected_sockets_;
            }
            for (auto& sock : socks_copy) {
                if (is_connected(sock) != Connection::Connected) {
                    logger.log(LogLevel::Debug, __func__, dataForLogger(type) +
                               "Socket " + sock.getAddressAndPort() + " disconnected");
                    disconnected(sock, true);
                }
            }
        } else if (type == ConnectionHandlerType::Client) {
            if (socket_client_.ptr && is_connected(socket_client_) != Connection::Connected) {
                logger.log(LogLevel::Info, __func__, dataForLogger(type) + "Connection lost");
                disconnect(false);
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    logger.log(LogLevel::Debug, __func__, dataForLogger(type) + "Connection check task finished");
}

void ConnectionHandler::runTask(ConnectedSocket& sock) {
    if (task_) {
        try {
            task_(sock);
        } catch (std::exception &e) {
            logger.log(LogLevel::Error, "task", dataForLogger(type) + e.what());
        }
    }
}