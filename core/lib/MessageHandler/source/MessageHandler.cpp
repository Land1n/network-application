//
// Created by ivan on 10.03.2026.
//
#include <memory>
#include <boost/json.hpp>

#include "MessageHandler.hpp"
#include "SignalMessage.hpp"
#include "InformationMessage.hpp"


namespace json = boost::json;

MessageHandler::MessageHandler(bool DEBUG) {
    creator_message->addMessageOnMap("signal",[](const std::string& t, json::value& v) { return std::make_unique<SignalMessage>(t, v); });
    creator_message->addMessageOnMap("information",[](const std::string& t, json::value& v) { return std::make_unique<InformationMessage>(t, v); });
    logger.init5Levels();
    if (!DEBUG)
        logger.setLogLevel("ERROR");
    else
        logger.setLogLevel("DEBUG");

}

std::unique_ptr<Message> MessageHandler::parse(const TransportMessage &transport_message) {

    std::string_view sv(reinterpret_cast<const char*>(transport_message.payload.data()),
                           transport_message.payload.size());
    json::value jv;
    try {
        jv = boost::json::parse(sv);
        logger("DEBUG") << "MessageHandler : Parse payload" << "\n";
    }catch (std::exception& e) {
        logger("ERROR") << "MessageHandler : Parse payload" << "\n";
        return nullptr;
    }
    auto message = creator_message->createMessage(transport_message.type,jv);
    logger("DEBUG") << "MessageHandler : Create Message" << "\n";
    if (message != nullptr) {
        message->setTransactionType();
    }
    return message;
}

TransportMessage MessageHandler::serialize(std::unique_ptr<Message> message) {
    if (!message) {
        return TransportMessage();
    }
    json::object payload;

    payload["type"] = message->type;
    if (message->type == "signal") {
        logger("DEBUG") << "MessageHandler : Serialize Message" << "\n";
        auto* signal_message = dynamic_cast<SignalMessage*>(message.get());

        if (!signal_message) return TransportMessage(message->type, {});

        payload["central_Freq"] = signal_message->getCentralFreq();
        json::array signal;
        for (const auto& c : signal_message->getSignal()) {
            signal.push_back({c.real(), c.imag()});
        }
        payload["signal"] = signal;

    } else if (message->type == "information") {
        logger("DEBUG") << "MessageHandler : Serialize Message" << "\n";
        auto* information_message = dynamic_cast<InformationMessage*>(message.get());
        if (!information_message) return TransportMessage(message->type, {});

        payload["numberCore"] = information_message->getNumberCore();
    } else {
        logger("WARN") << "MessageHandler : Serialize Message" << "\n";
        return TransportMessage(message->type, {});
    }
    std::string json_str = json::serialize(payload);
    std::vector<uint8_t> bytes(json_str.begin(),json_str.end());
    TransportMessage transport_message(message->type,bytes);
    return transport_message;
}
