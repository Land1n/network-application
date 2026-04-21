//
// Created by ivan on 09.03.2026.
//
//
#pragma once

#include "../../clientserveriface/include/clientserveriface/server.h"
#include "ConnectionHandler.hpp"
#include "MessageHandler.hpp"
#include "TransportHandler.hpp"
#include "ServerRequestResponseHandler.hpp"
#include "Logger.hpp"

#include <unordered_map>
#include <mutex>
// TODO: multiConnect
class SegmentServer : public Network::Server {
public:
    SegmentServer(const std::string& address, int port, bool debug = false);
    ~SegmentServer() override;

    void start() override;
    void stop() override;

    bool isRunning();

    void write(Network::ConnectionId id, const void *data, size_t sz) override;
    void disconnect(Network::ConnectionId id) override;
    void setIdDistributionHandler(IdDistributionHandler h) override;
private:
    const std::string address;
    const int port;
    Logger& logger = Logger::getInstance();


    // Обработчики
    std::unique_ptr<ConnectionHandler> connection_handler;
    std::unique_ptr<TransportHandler> transport_handler;
    std::unique_ptr<MessageHandler> message_handler;
    std::unique_ptr<ServerRequestResponseHandler> request_response_handler;

    Worker worker_task;
};