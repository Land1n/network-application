//
// Created by ivan on 10.03.2026.
//

#include "InformationMessage.hpp"

InformationMessage::InformationMessage(const std::string& type,Transaction transaction, int numberCore)
    : Message(type,transaction), _numberCore(numberCore) {}

int InformationMessage::getNumberCore() const {
    return _numberCore;
}

InformationMessage::InformationMessage(const std::string& type,Transaction transaction,json::value &jv) : Message(type,transaction) {
    this->_numberCore = jv.at("numberCore").as_int64();
}