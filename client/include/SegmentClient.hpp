//
// Created by ivan on 14.03.2026.
//
#pragma once

#include "../../clientserveriface/include/clientserveriface/client.h"
#include "ConnectionHandler.hpp"
#include "Logger.hpp"
#include "MessageHandler.hpp"
#include "TransportHandler.hpp"

#include <mutex>
#include <memory>
#include <functional>
#include <atomic>

class SegmentClient : public Network::Client {
public:
    SegmentClient(const std::string& serverAddress, int serverPort, bool debug = false);
    ~SegmentClient() override;

    void start() override;
    void stop() override;

    void connect();
    void disconnect() override;

    void write(const void* data, size_t sz) override;

    void setCloseConnectionHandler(ConnChangeHandler h) override;
    void setNewConnectionHandler(ConnChangeHandler h) override;
    void setReadHandler(ReadHandler h) override;

    bool isRunning() const { return connectionHandler && connectionHandler->getIsWork(); }

private:
    std::string serverAddress;
    int serverPort;
    bool debug;

    std::shared_ptr<ConnectionHandler> connectionHandler;
    Logger& logger = Logger::getInstance();

    ReadHandler readHandler;
    ConnChangeHandler closeHandler;
    ConnChangeHandler newHandler;

    std::mutex writeMutex;
};