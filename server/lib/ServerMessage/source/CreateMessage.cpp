//
// Created by ivan on 10.03.2026.
//
#include "CreateMessage.hpp"

std::unique_ptr<Message> createMessage(const std::string& type,json::value &jv) {
    static const std::unordered_map<std::string,
        std::function<std::unique_ptr<Message>(const std::string&, json::value&)>> factory = {
        {"signal",      [](const std::string& t, json::value& v) { return std::make_unique<SignalMessage>(t, v); }},
        {"information", [](const std::string& t, json::value& v) { return std::make_unique<InformationMessage>(t, v); }}
    };
    auto it = factory.find(type);
    if (it != factory.end()) {
        return it->second(type,jv);
    }
    return nullptr;
}