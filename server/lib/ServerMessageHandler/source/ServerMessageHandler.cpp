//
// Created by ivan on 10.03.2026.
//
#include "ServerMessageHandler.hpp"

#include <iostream>
#include <memory>
#include <boost/json.hpp>

#include "TransportMessage.hpp"

#include "Message.hpp"
#include "SignalMessage.hpp"
#include "InformationMessage.hpp"

#include "CreateMessage.hpp"

namespace json = boost::json;

std::unique_ptr<Message> ServerMessageHandler::parse(const TransportMessage &transport_message) {

    std::string_view sv(reinterpret_cast<const char*>(transport_message.payload.data()),
                           transport_message.payload.size());
    json::value jv;
    try {
        jv = boost::json::parse(sv);
    }catch (std::exception& e) {
        return nullptr;
    }

    return createMessage(transport_message.type,jv);
}

TransportMessage ServerMessageHandler::serialize(std::unique_ptr<Message> message) {
    if (!message) {
        return TransportMessage();
    }
    json::object payload;
    if (message->type == "signal") {
        auto* signal_message = dynamic_cast<SignalMessage*>(message.get());

        if (!signal_message) return TransportMessage(message->type, {});

        payload["central_Freq"] = signal_message->getCentralFreq();
        json::array signal;
        for (const auto& c : signal_message->getSignal()) {
            signal.push_back({c.real(), c.imag()});
        }
        payload["signal"] = signal;

    } else if (message->type == "information") {
        auto* information_message = dynamic_cast<InformationMessage*>(message.get());
        if (!information_message) return TransportMessage(message->type, {});

        payload["numberCore"] = information_message->getNumberCore();
    } else {
        return TransportMessage(message->type, {});
    }
    std::string json_str = json::serialize(payload);
    std::vector<uint8_t> bytes(json_str.begin(),json_str.end());
    TransportMessage transport_message(message->type,bytes);
    return transport_message;
}
