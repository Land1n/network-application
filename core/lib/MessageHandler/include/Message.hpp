//
// Created by ivan on 08.03.2026.
//
#pragma once
#include <string>

#include "Transaction.hpp"

class  Message {
public:
    std::string type;
    Transaction transaction;
    Message(const std::string &type) : type(type),transaction(Transaction::Tests) {}
    Message(const std::string &type,Transaction transaction) : type(type),transaction(transaction) {}
    virtual ~Message() = default;
};