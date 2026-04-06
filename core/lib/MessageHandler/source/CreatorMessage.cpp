//
// Created by ivan on 11.03.2026.
//

#include "CreatorMessage.hpp"

#include <iostream>
#include <utility>


std::unique_ptr<Message> CreatorMessage::createMessage(const std::string& type, Transaction transaction, json::value &json_value) {
    auto it = factory.find(type);
    try {
        if (it != factory.end()) {
            return it->second(type, transaction, json_value);
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return nullptr;
}

bool CreatorMessage::addMessageOnMap(const std::string& type,
                                     std::function<std::unique_ptr<Message>(const std::string&, Transaction, json::value&)> function) {
    try {
        factory[type] = std::move(function);
        return true;
    } catch (...) {
        return false;
    }
}

CreatorMessage::CreatorMessage(std::unordered_map<std::string, std::function<std::unique_ptr<Message>(const std::string&, Transaction, json::value&)>> factory)
    : factory(std::move(factory))
{}