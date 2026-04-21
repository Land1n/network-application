//
// Created by ivan on 08.03.2026.
//
#pragma once
#include <string>
#include <boost/json.hpp>

#include "Transaction.hpp"

namespace json = boost::json;

class  Message {
public:
    std::string type;
    Transaction transaction;
    Message(const std::string &type) : type(type),transaction(Transaction::Tests) {}
    Message(const std::string &type,Transaction transaction) : type(type),transaction(transaction) {}
    virtual json::object serialize() {
        json::object payload;
        payload["type"] = type;
        if (transaction == Transaction::Request) payload["transaction"] = 0;
        else if (transaction == Transaction::Response) payload["transaction"] = 1;
        else payload["transaction"] = -1;
        return payload;
    };
    virtual ~Message() = default;
};