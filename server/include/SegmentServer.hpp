//
// Created by ivan on 09.03.2026.
//
//
#pragma once

// #include <atomic>
// #include <unordered_map>
// #include <thread>
// #include <mutex>
// #include <memory>
//
#include "../../clientserveriface/include/clientserveriface/server.h"
#include "ConnectionHandler.hpp"
// #include "Logger.hpp"
#include "LoggerFactory.hpp"
#include "MessageHandler.hpp"
// #include "ServerRequestResponseHandler.hpp"
//
// class SegmentServer : public Network::Server {
// public:
//     SegmentServer(const std::string& address, int port, bool debug = false);
//     ~SegmentServer() override;
//
//     // Network::Server interface
//     void start() override;
//     void stop() override;
//     void write(Network::ConnectionId id, const void* data, size_t sz) override;
//     void disconnect(Network::ConnectionId id) override;
//
// private:
//     struct ConnectionContext {
//         std::shared_ptr<tcp::socket> socket;
//         std::shared_ptr<std::thread> readThread;
//         bool stopReading = false;
//     };
//
//     void startReading(Network::ConnectionId id, std::shared_ptr<tcp::socket> socket);
//     void onConnectionClosed(Network::ConnectionId id);
//
//     std::string address_;
//     int port_;
//     bool debug_;
//     std::shared_ptr<ConnectionHandler> connHandler_;
//     std::shared_ptr<Logger> logger_;
//     std::atomic<bool> isRunning_{false};
//
//     // Active connections
//     std::unordered_map<Network::ConnectionId, ConnectionContext> connections_;
//     std::mutex connectionsMutex_;
//
//     // ID generation
//     std::atomic<Network::ConnectionId> nextId_{0};
//     Network::Server::IdDistributionHandler idDistHandler_;
//
//     // Message handling (optional)
//     std::shared_ptr<MessageHandler> msgHandler_;
//     std::shared_ptr<ServerRequestResponseHandler> rrHandler_;
//     bool autoHandleRequests_ = false;
// };

class SegmentServer : public Network::Server {
public:
    SegmentServer(const std::string& address, int port, bool debug = false);
    ~SegmentServer();

    void start() override;
    void stop() override;
    void write(Network::ConnectionId id, const void *data, size_t sz) override;
    void disconnect(Network::ConnectionId id) override;
private:
    const std::string address;
    const int port;

    std::shared_ptr<ConnectionHandler> connectionHandler = nullptr;
    std::shared_ptr<Logger> logger = nullptr;
};