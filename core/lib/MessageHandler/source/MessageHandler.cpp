//
// Created by ivan on 10.03.2026.
//
#include <memory>
#include <boost/json.hpp>

#include "MessageHandler.hpp"
#include "SignalMessage.hpp"
#include "InformationMessage.hpp"
#include <boost/json.hpp>

#include "RawMessage.hpp"

namespace json = boost::json;

MessageHandler::MessageHandler(bool DEBUG) {
    creator_message->addMessageOnMap("signal", [](const std::string& type, Transaction transaction, json::value& jv) {
        return std::make_unique<SignalMessage>(type, transaction, jv);
    });
    creator_message->addMessageOnMap("information", [](const std::string& type, Transaction transaction, json::value& jv) {
        return std::make_unique<InformationMessage>(type, transaction, jv);
    });
    creator_message->addMessageOnMap("raw", [](const std::string& type, Transaction transaction, json::value& jv) {
        return std::make_unique<RawMessage>(type, transaction, jv);
    });
    if (!DEBUG) logger.setLevel(LogLevel::Error);
}

std::unique_ptr<Message> MessageHandler::parse(const TransportMessage &transport_message) {
    const std::string sv(transport_message.payload.begin(), transport_message.payload.end());
    json::value jv;
    try {
        jv = boost::json::parse(sv);
        logger.log(LogLevel::Debug, __func__, "Successful");
    } catch (std::exception& e) {
        logger.log(LogLevel::Critical, __func__, std::string("Parse error: ") + e.what());
        return nullptr;
    }
    auto message = creator_message->createMessage(transport_message.type, transport_message.transaction, jv);
    logger.log(LogLevel::Debug, __func__, "Create Message");
    return message;
}

TransportMessage MessageHandler::serialize(std::unique_ptr<Message> message) {
    if (!message) return TransportMessage();
    std::string json_str = json::serialize(message->serialize());
    std::vector<uint8_t> bytes(json_str.begin(), json_str.end());
    return TransportMessage(message->type, message->transaction, bytes);
}