//
// Created by ivan on 09.03.2026.
//
//
#pragma once

#include "../../clientserveriface/include/clientserveriface/server.h"
#include "ConnectionHandler.hpp"
#include "MessageHandler.hpp"
#include "TransportHandler.hpp"
#include "LoggerFactory.hpp"

#include <unordered_map>
#include <mutex>

class SegmentServer : public Network::Server {
public:
    SegmentServer(const std::string& address, int port, bool debug = false);
    ~SegmentServer();

    void start() override;
    void stop() override;
    void write(Network::ConnectionId id, const void *data, size_t sz) override;
    void disconnect(Network::ConnectionId id) override;
    void setIdDistributionHandler(IdDistributionHandler h) override;
    void setCloseConnectionHandler(ConnChangeHandler h) override;
    void setNewConnectionHandler(ConnChangeHandler h) override;
    void setReadHandler(ReadHandler h) override;

private:
    const std::string address;
    const int port;

    std::shared_ptr<ConnectionHandler> connectionHandler;
    std::shared_ptr<Logger> logger;
    std::shared_ptr<MessageHandler> messageHandler; // может пригодиться для других типов сообщений

    ReadHandler readHandler;
    ConnChangeHandler closeHandler;
    ConnChangeHandler newHandler;

    std::unordered_map<Network::ConnectionId, std::mutex> socketMutexes;
    std::mutex socketMutexesMutex;
};