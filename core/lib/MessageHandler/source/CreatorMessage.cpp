//
// Created by ivan on 11.03.2026.
//

#include "CreatorMessage.hpp"

#include <utility>


std::unique_ptr<Message> CreatorMessage::createMessage(std::string type,json::value &json_value) {

    auto it = factory.find(type);
    if (it != factory.end()) {
        return it->second(type,json_value);
    }
    return nullptr;
}

bool CreatorMessage::addMessageOnMap(std::string type,std::function<std::unique_ptr<Message>(const std::string&, json::value&)> function) {
    try {
        factory[type] = std::move(function);
        if (factory.at(type))
            return true;
        return  false;
    } catch (std::exception &e) {
        return false;
    }
}

CreatorMessage::CreatorMessage(std::unordered_map<std::string, std::function<std::unique_ptr<Message>(const std::string &, json::value &)> > factory)
    : factory(std::move(factory))
{}
