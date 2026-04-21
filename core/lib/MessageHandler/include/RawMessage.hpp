//
// Created by ivan on 17.04.2026.
//

#pragma once

#include <string>
#include "Message.hpp"

#include "boost/json.hpp"

namespace json = boost::json;

class RawMessage : public Message {
public:
    RawMessage(const std::string &type, Transaction transaction, const std::vector<uint8_t> &data);
    RawMessage(const std::string &type,Transaction transaction, json::value &jv);

    json::object serialize() override;

    std::vector<uint8_t> getData() const;
private:
    std::vector<uint8_t> data_;
};