//
// Created by ivan on 07.03.2026.
//

#pragma once
#include <string>
#include <vector>

struct
TransportMessage {
    TransportMessage() {};
    TransportMessage(std::vector<uint8_t> &payload) : type(""), payload(payload) {};
    TransportMessage(std::string &type, std::vector<uint8_t> payload) : type(type), payload(payload) {};
    std::string type;
    std::vector<uint8_t> payload;
};
