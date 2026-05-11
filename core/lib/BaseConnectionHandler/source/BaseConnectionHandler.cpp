#include "BaseConnectionHandler.hpp"

BaseConnectionHandler::BaseConnectionHandler(const std::string& address, int port)
    : address(address), port(port)  {
    generateID = []() { return defaultGenerator.generate(); };
}

BaseConnectionHandler::~BaseConnectionHandler() = default;

std::shared_ptr<tcp::socket> BaseConnectionHandler::createSocket() {
	return std::make_shared<tcp::socket>(io_context);
}

Connection BaseConnectionHandler::is_connected(const ConnectedSocket& sock) {
    if (!sock.ptr || !sock.ptr->is_open())
        return Connection::Disconnected;

    try {
        sock.ptr->non_blocking(true);
        boost::system::error_code ec;
        char c;
        sock.ptr->receive(boost::asio::buffer(&c, 1), boost::asio::socket_base::message_peek, ec);
        sock.ptr->non_blocking(false);
        if (ec == boost::asio::error::would_block)
            return Connection::Connected;
        if (ec == boost::asio::error::eof)
            return Connection::Disconnected;
        return Connection::Disconnected;
    } catch (std::exception& e) {
        logger.log(LogLevel::Warn, __func__, e.what());
        return Connection::Error;
    }
}

void BaseConnectionHandler::runTask(ConnectedSocket& sock) {
    if (task_) {
        try {
            task_(sock);
        } catch (std::exception& e) {
            logger.log(LogLevel::Error, "task", e.what());
        }
    }
}

void BaseConnectionHandler::setTaskSocket(std::function<void(ConnectedSocket&)> task) {
    task_ = std::move(task);
}

void BaseConnectionHandler::setGenerateID(std::function<size_t()> g) {
    generateID = std::move(g);
}

void BaseConnectionHandler::setNewConnectionHandler(std::function<void(ConnectedSocket)> h) {
    newConnection = std::move(h);
}

void BaseConnectionHandler::setCloseConnectionHandler(std::function<void(ConnectedSocket)> h) {
    closeConnection = std::move(h);
}


bool BaseConnectionHandler::getIsWork() const {
    return isWork.load();
}
