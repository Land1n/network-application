//
// Created by ivan on 10.03.2026.
//
#include <memory>
#include <boost/json.hpp>

#include "MessageHandler.hpp"
#include "SignalMessage.hpp"
#include "InformationMessage.hpp"
#include <boost/json.hpp>

namespace json = boost::json;

MessageHandler::MessageHandler(bool DEBUG) {
    creator_message->addMessageOnMap("signal", [](const std::string& type, Transaction transaction, json::value& jv) {
        return std::make_unique<SignalMessage>(type, transaction, jv);
    });
    creator_message->addMessageOnMap("information", [](const std::string& type, Transaction transaction, json::value& jv) {
        return std::make_unique<InformationMessage>(type, transaction, jv);
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
        logger.log(LogLevel::Error, __func__, std::string("Parse error: ") + e.what());
        return nullptr;
    }
    auto message = creator_message->createMessage(transport_message.type, transport_message.transaction, jv);
    logger.log(LogLevel::Debug, __func__, "Create Message");
    return message;
}

TransportMessage MessageHandler::serialize(std::unique_ptr<Message> message) {
    if (!message) return TransportMessage();
    json::object payload;
    payload["type"] = message->type;
    if (message->transaction == Transaction::Request) payload["transaction"] = 0;
    else if (message->transaction == Transaction::Response) payload["transaction"] = 1;
    else payload["transaction"] = -1;

    if (message->type == "signal" && message->transaction == Transaction::Response) {
        logger.log(LogLevel::Debug, __func__, "Serialize SignalMessage");
        auto* signal_message = dynamic_cast<SignalMessage*>(message.get());
        if (!signal_message) return TransportMessage(message->type, message->transaction, {});
        payload["central_Freq"] = signal_message->getCentralFreq();
        json::array signal;
        for (const auto& c : signal_message->getSignal())
            signal.push_back({c.real(), c.imag()});
        payload["signal"] = signal;
    } else if (message->type == "information" && message->transaction == Transaction::Response) {
        logger.log(LogLevel::Debug, __func__, "Serialize InformationMessage");
        auto* info_message = dynamic_cast<InformationMessage*>(message.get());
        if (!info_message) return TransportMessage(message->type, message->transaction, {});
        payload["numberCore"] = info_message->getNumberCore();
    }
    std::string json_str = json::serialize(payload);
    std::vector<uint8_t> bytes(json_str.begin(), json_str.end());
    return TransportMessage(message->type, message->transaction, bytes);
}