//
// Created by ivan on 10.03.2026.
//

#include "InformationMessage.hpp"

InformationMessage::InformationMessage(const std::string& type, int numberCore)
    : Message(type), _numberCore(numberCore) {}

int InformationMessage::getNumberCore() const {
    return _numberCore;
}

InformationMessage::InformationMessage(const std::string& type,json::value &jv) : Message(type) {
    this->_numberCore = jv.at("numberCore").as_int64();
}

void InformationMessage::setTransactionType() {
    if (_numberCore == 0)
        this->transactionType = TransactionType::Request;
    else
        this->transactionType = TransactionType::Response;
}