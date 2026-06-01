//
// Created by ivan on 09.03.2026.
//
//
#pragma once

#include "../../clientserveriface/include/clientserveriface/server.h"
#include "MessageHandler.hpp"
#include "../../core/lib/TransportHandler/include/TransportHandler/TransportHandler.hpp"
#include "ServerRequestResponseHandler.hpp"
#include "Logger.hpp"
#include "SyncServerConnectionHandler.hpp"

#include <unordered_map>
#include <mutex>
// + TODO:multiConnect
class SegmentServer : public Network::Server {
public:
    SegmentServer(const std::string& address, int port,bool multiConnect);
    ~SegmentServer() override;

    void start() override;
    void stop() override;

    bool isRunning();

    void write(Network::ConnectionId id, const void *data, size_t sz) override;
    void disconnect(Network::ConnectionId id) override;
    void setIdDistributionHandler(IdDistributionHandler h) override;
    int getAliveThreads();
	std::vector<ConnectedSocket> getConnectedClients();
private:
    const std::string address;
    const int port;
    Logger& logger = Logger::getInstance();
    // Обработчики
    std::unique_ptr<SyncServerConnectionHandler> connection_handler;
    std::unique_ptr<MessageHandler> message_handler;
    std::unique_ptr<ServerRequestResponseHandler> request_response_handler;

    std::atomic<int> aliveThreads = 0;

    bool multiConnect;
};