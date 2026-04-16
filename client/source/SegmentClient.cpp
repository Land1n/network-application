#include "SegmentClient.hpp"
#include <boost/json.hpp>
#include <thread>
#include <chrono>
#include <iostream>

SegmentClient::SegmentClient(const std::string& serverAddress, int serverPort, bool debug)
    : serverAddress(serverAddress), serverPort(serverPort), debug(debug)
{
    connectionHandler = std::make_shared<ConnectionHandler>(
        serverAddress, serverPort, ConnectionHandlerType::Client, !debug
    );
    logger.setLevel(debug ? LogLevel::Debug : LogLevel::Info);
}

SegmentClient::~SegmentClient() {
    stop();
}

void SegmentClient::start() {
    if (!connectionHandler->getIsWork()) {
        connectionHandler->start();
        logger.log(LogLevel::Info, "start", "Client handler started");
    }
}

void SegmentClient::stop() {
    logger.log(LogLevel::Info, "stop", "Stopping client...");
    disconnect();
    connectionHandler->stop();
    logger.log(LogLevel::Info, "stop", "Client stopped");
}

void SegmentClient::connect() {
    if (!connectionHandler->getIsWork()) {
        start();
    }
    auto sock = connectionHandler->connect();
    if (sock.ptr) {
        logger.log(LogLevel::Info, "connect", "Connected to " + serverAddress + ":" + std::to_string(serverPort));
        if (newHandler) newHandler();
    } else {
        logger.log(LogLevel::Error, "connect", "Connection failed");
    }
}

void SegmentClient::disconnect() {
    if (connectionHandler->getIsWork()) {
        connectionHandler->disconnect(true);
        if (closeHandler) closeHandler();
        logger.log(LogLevel::Info, "disconnect", "Disconnected");
    }
}

void SegmentClient::write(const void* data, size_t sz) {
    auto sock = connectionHandler->getSocket();
    std::cout << !sock.ptr->is_open() << std::endl;
    if (!sock.ptr || !sock.ptr->is_open()) {
        logger.log(LogLevel::Warn, "write", "Socket not open");
        return;
    }

    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    boost::json::array arr;
    arr.reserve(sz);
    for (size_t i = 0; i < sz; ++i) {
        arr.push_back(bytes[i]);
    }

    boost::json::object obj;
    obj["type"] = "raw";
    obj["transaction"] = static_cast<int>(Transaction::Request);
    obj["data"] = std::move(arr);

    std::string jsonStr = boost::json::serialize(obj);
    std::vector<uint8_t> payload(jsonStr.begin(), jsonStr.end());
    TransportMessage tm("raw", Transaction::Request, payload);

    std::lock_guard<std::mutex> lock(writeMutex);
    TransportHandler transport(sock.ptr);
    if (!transport.write(tm)) {
        logger.log(LogLevel::Error, "write", "Failed to send data");
    }
}

void SegmentClient::setCloseConnectionHandler(ConnChangeHandler h) {
    closeHandler = std::move(h);
    connectionHandler->setCloseConnectionHandler([this](ConnectedSocket) {
        if (closeHandler) closeHandler();
    });
}

void SegmentClient::setNewConnectionHandler(ConnChangeHandler h) {
    newHandler = std::move(h);
    connectionHandler->setNewConnectionHandler([this](ConnectedSocket) {
        if (newHandler) newHandler();
    });
}

void SegmentClient::setReadHandler(ReadHandler h) {
    readHandler = std::move(h);
}