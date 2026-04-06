#pragma once

#include <memory>
#include "boost/json.hpp"
#include <unordered_map>
#include <functional>
#include "Message.hpp"
#include "Transaction.hpp"

namespace json = boost::json;

class CreatorMessage {
public:
    CreatorMessage() {};
    CreatorMessage(std::unordered_map<std::string, std::function<std::unique_ptr<Message>(const std::string&, Transaction, json::value&)>> factory);

    std::unique_ptr<Message> createMessage(const std::string& type, Transaction transaction, json::value &json_value);

    bool addMessageOnMap(const std::string& type, std::function<std::unique_ptr<Message>(const std::string&, Transaction, json::value&)> function);

private:
    std::unordered_map<std::string, std::function<std::unique_ptr<Message>(const std::string&, Transaction, json::value&)>> factory;
};