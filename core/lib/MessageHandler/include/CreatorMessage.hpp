//
// Created by ivan on 11.03.2026.
//

#pragma once

#include <memory>
#include "boost/json.hpp"
#include <unordered_map>
#include <functional>
#include "Message.hpp"

namespace json = boost::json;

class CreatorMessage {
public:
    CreatorMessage() {};
    CreatorMessage(std::unordered_map<std::string, std::function<std::unique_ptr<Message>(const std::string&, json::value&)>> factory);

    std::unique_ptr<Message> createMessage(std::string type,json::value &json_value);

    bool addMessageOnMap(std::string type, std::function<std::unique_ptr<Message>(const std::string &, json::value &)> function);
    std::unordered_map<std::string, std::function<std::unique_ptr<Message>(const std::string&, json::value&)>> getFactory();
private:
    std::unordered_map<std::string, std::function<std::unique_ptr<Message>(const std::string&, json::value&)>> factory;

};