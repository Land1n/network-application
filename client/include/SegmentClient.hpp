//
// Created by ivan on 14.03.2026.
//
#pragma once

#include "../../clientserveriface/include/clientserveriface/client.h"
#include "ConnectionHandler.hpp"
#include "Logger.hpp"
#include "MessageHandler.hpp"
#include "TransportHandler.hpp"
#include "ClientRequestResponseHandler.hpp"

#include <mutex>
#include <memory>
#include <functional>
#include <atomic>

class SegmentClient : public Network::Client {
public:
    SegmentClient(std::string  address, int port, bool debug = false);
    ~SegmentClient() override;

    void start() override;
    void stop() override;

    void connect();
    void disconnect() override;

    void write(const void* data, size_t sz) override;

    bool isRunning() const;

private:
    std::string address;
    int port;
    bool debug;

    // Обработчики
    std::unique_ptr<ConnectionHandler> connection_handler;
    std::unique_ptr<TransportHandler> transport_handler;
    std::unique_ptr<MessageHandler> message_handler;
    std::unique_ptr<ClientRequestResponseHandler> request_response_handler;

    Logger& logger = Logger::getInstance();
    Worker worker_task;
};