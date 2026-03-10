//
// Created by ivan on 08.03.2026.
//
#pragma once
#include <string>

class  Message {
public:
    std::string type;
    Message(std::string type) : type(type) {}
    virtual ~Message() = default;
};