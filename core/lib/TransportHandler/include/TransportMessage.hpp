//
// Created by ivan on 07.03.2026.
//

#pragma once
#include <string>
#include <vector>

#include "Transaction.hpp"

struct TransportMessage {
    TransportMessage() {};
    TransportMessage(const std::string &type,Transaction transaction, std::vector<uint8_t> payload) : type(type),transaction(transaction), payload(payload) {};
    std::string type;
    Transaction transaction;
    std::vector<uint8_t> payload;
};
