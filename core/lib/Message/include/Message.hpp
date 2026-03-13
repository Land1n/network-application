//
// Created by ivan on 08.03.2026.
//
#pragma once
#include <string>

enum class TransactionType {
    Request = 0,
    Response = 1
};

class  Message {
public:
    std::string type;
    TransactionType transactionType;
    Message(std::string type) : type(type) {}
    virtual ~Message() = default;
    virtual void setTransactionType();
};