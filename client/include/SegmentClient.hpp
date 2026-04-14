//
// Created by ivan on 14.03.2026.
//
#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <chrono>

#include "../../clientserveriface/include/clientserveriface/client.h"
#include "ConnectionHandler.hpp"
#include "Logger.hpp"
#include "LoggerFactory.hpp"
#include "MessageHandler.hpp"
#include "TransportHandler.hpp"

// class SegmentClient : public Network::Client {
// public:
//     SegmentClient(const std::string& hostname, int port, bool debug = false, uint32_t reconnectTimeoutMs = 0);
//     ~SegmentClient() override;
//
//     void start() override;
//     void stop() override;
//     void write(const void* data, size_t sz) override;
//     void disconnect() override;
//
// private:
//     void runReadLoop();
//     void scheduleReconnect();
//     void doConnect();
//
//     std::string hostname_;
//     int port_;
//     bool debug_;
//     uint32_t reconnectTimeoutMs_;
//     std::shared_ptr<ConnectionHandler> connHandler_;
//     std::shared_ptr<Logger> logger_;
//     std::atomic<bool> isRunning_{false};
//     std::atomic<bool> shouldReconnect_{false};
//
//     std::shared_ptr<std::thread> readThread_;
//     std::shared_ptr<std::thread> reconnectThread_;
//     std::mutex mutex_;
//
//     std::shared_ptr<MessageHandler> msgHandler_;
// };