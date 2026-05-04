#include "SegmentClient.hpp"
#include <boost/json.hpp>
#include <thread>
#include <chrono>
#include <iostream>
#include <random>
#include <utility>

#include "InformationMessage.hpp"
#include "RawMessage.hpp"
#include "SignalMessage.hpp"
#include "../lib/ClientRequestResponseHandler/include/ClientRequestResponseHandler.hpp"


SegmentClient::SegmentClient(std::string serverAddress, int port, bool debug)
    : address(std::move(serverAddress)), port(port), debug(debug) {
    connection_handler = std::make_unique<ConnectionHandler>(
        address, port, ConnectionHandlerType::Client, true
    );
    message_handler = std::make_unique<MessageHandler>();
    request_response_handler = std::make_unique<ClientRequestResponseHandler>(message_handler->creator_message);
    logger.setLevel(LogLevel::Debug);
}

SegmentClient::~SegmentClient() {
    stop();
}

void SegmentClient::start() {
    connection_handler->start();

    connection_handler->setTaskSocket([this](ConnectedSocket &cs) {
        transport_handler = std::make_unique<TransportHandler>(cs.ptr);
        transport_handler->setOnReadHandler([this ](size_t id, const void *data, size_t data_sz) {
            if (readHandler) readHandler(data, data_sz);
        });
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(1, 3);
        while (connection_handler->getIsWork()) {
            if (worker_task.getSizeQueue() == 0)
            worker_task.addTask([this,&dist,&gen]() {
                std::string new_message_type;
                int val = dist(gen);
                if (val == 1) new_message_type = "signal";
                else if (val == 2) new_message_type = "information";
                else if (val == 3) new_message_type = "raw";
                auto transport_message = message_handler->serialize(
                    std::make_unique<Message>(new_message_type, Transaction::Request)
                );
                logger.log(LogLevel::Info, "Auto task", "Generation message type: " + new_message_type);
                if (transport_handler->write(transport_message)) {
                    TransportMessage new_transport_message = transport_handler->read();
                    if (new_transport_message.transaction != Transaction::Error) {
                        auto new_message = message_handler->parse(new_transport_message);
                        if (new_message) {
                            if (new_message->type == "signal") {
                                auto *new_message_signal = dynamic_cast<SignalMessage *>(new_message.get());
                                if (new_message_signal != nullptr) {
                                    logger.log(LogLevel::Info, "Auto task","Signal received");
                                    for (const auto &sample: new_message_signal->getSignal()) {
                                        std::cout << sample << " ";
                                    }
                                    std::cout << std::endl;
                                }
                            } else if (new_message->type == "information") {
                                auto *new_message_information = dynamic_cast<InformationMessage *>(new_message.get());
                                if (new_message_information != nullptr) {
                                    logger.log(LogLevel::Info, "Auto task","Information received");
                                    std::cout << new_message_information->getNumberCore() << std::endl;
                                }
                            } else if (new_message->type == "raw") {
                                auto *new_message_raw = dynamic_cast<RawMessage *>(new_message.get());
                                if (new_message_raw != nullptr) {
                                    logger.log(LogLevel::Info, "Auto task","Raw received");
                                    std::cout << reinterpret_cast<const char *>(new_message_raw->getData().data()) << std::endl;
                                }
                            } else {
                                logger.log(LogLevel::Error, "Auto task", "Unknown message type");
                            }
                        }
                    }
                }
                std::this_thread::sleep_for(std::chrono::seconds(2));
            });
        }
    });
}

void SegmentClient::stop() {
    logger.log(LogLevel::Info, "stop", "Stopping client...");
    disconnect();
    connection_handler->stop();
    worker_task.stop(true);
    logger.log(LogLevel::Info, "stop", "Client stopped");
}

void SegmentClient::connect() {
    auto sock = connection_handler->connect();
    if (sock.ptr) {
        logger.log(LogLevel::Info, "connect", "Connected to " + address + ":" + std::to_string(port));
        if (newHandler) newHandler();
    } else {
        logger.log(LogLevel::Error, "connect", "Connection failed");
    }
}

void SegmentClient::disconnect() {
    if (connection_handler->getIsWork()) {
        connection_handler->disconnect(true);
        if (closeHandler) closeHandler();
        logger.log(LogLevel::Info, "disconnect", "Disconnected");
    }
}

void SegmentClient::write(const void *data, size_t sz) {
    auto sock = connection_handler->getSocket();
    if (!sock.ptr || !sock.ptr->is_open()) {
        logger.log(LogLevel::Critical, __func__, "Write fail");
        logger.log(LogLevel::Critical, __func__,
                   "connection_handler->getIsWork() = " + std::to_string(connection_handler->getIsWork()));
        return;
    }

    const uint8_t *bytes = static_cast<const uint8_t *>(data);
    std::vector<uint8_t> payload(bytes, bytes + sz);

    worker_task.addTask([this,payload]() {
        auto raw_msg = std::make_unique<RawMessage>("raw", Transaction::Response, payload);
        logger.log(LogLevel::Info, "User task", "Sending raw message");
        TransportMessage transport_message = message_handler->serialize(std::move(raw_msg));
        transport_handler->write(transport_message);
        TransportMessage new_transport_message = transport_handler->read();
        if (new_transport_message.transaction != Transaction::Error) {
            auto new_message = message_handler->parse(new_transport_message);
            if (new_message) {
                if (new_message->type == "raw") {
                    auto *new_message_raw = dynamic_cast<RawMessage *>(new_message.get());
                    if (new_message_raw != nullptr) {
                        logger.log(LogLevel::Info, "User task","Raw received");
                        std::cout << reinterpret_cast<const char *>(new_message_raw->getData().data()) << std::endl;
                    }
                }
            }
        }
    });
}

bool SegmentClient::isRunning() const {
    return connection_handler->getIsWork();
}
