//
// Created by ivan on 17.04.2026.
//
#include "RawMessage.hpp"

#include <iostream>

RawMessage::RawMessage(const std::string &type, Transaction transaction, const std::vector<uint8_t>& data)
    : Message(type,transaction),data_(data) {}


RawMessage::RawMessage(const std::string &type, Transaction transaction, json::value &jv) : Message(type,transaction) {
    try {
        if (transaction == Transaction::Response) {
            auto& dataArray = jv.at("data").as_array();
            for (auto const& value : dataArray) {
                if (!value.is_int64() && !value.is_uint64()) {
                    std::cerr << "Value is not integer, kind=" << value.kind() << ", value=" << json::serialize(value) << std::endl;
                }
                data_.push_back(static_cast<uint8_t>(value.as_int64()));
            }
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

json::object RawMessage::serialize() {
    json::object payload;
    payload["type"] = type;

    if (transaction == Transaction::Request) payload["transaction"] = 0;
    else if (transaction == Transaction::Response) payload["transaction"] = 1;
    else payload["transaction"] = -1;

    json::array array;
    int i = 0;
    for (auto const& value : data_) {
        array.push_back(static_cast<uint8_t>(value));
    }
    payload["data"] = array;
    json::value value = payload;
    return payload;
}

std::vector<uint8_t> RawMessage::getData() const {
    return data_;
}

