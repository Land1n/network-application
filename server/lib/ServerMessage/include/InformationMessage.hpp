//
// Created by ivan on 10.03.2026.
//

#pragma once
#include <string>
#include "Message.hpp"

#include "boost/json.hpp"

namespace json = boost::json;

class InformationMessage : public Message {
public:
    InformationMessage(const std::string &type, int numberCore);
    InformationMessage(const std::string &type, json::value &jv);

    void setTransactionType() override;

    int getNumberCore() const;
private:
    int _numberCore;
};