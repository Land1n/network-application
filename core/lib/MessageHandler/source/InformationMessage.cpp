//
// Created by ivan on 10.03.2026.
//

#include "InformationMessage.hpp"

InformationMessage::InformationMessage(const std::string &type, Transaction transaction, int numberCore)
    : Message(type, transaction), _numberCore(numberCore) {
}

int InformationMessage::getNumberCore() const {
    return _numberCore;
}

InformationMessage::InformationMessage(const std::string &type, Transaction transaction, json::value &jv) : Message(
    type, transaction) {
    if (transaction == Transaction::Response)
        this->_numberCore = jv.at("numberCore").as_int64();
}

json::object InformationMessage::serialize() {
    json::object payload = json::object();
    payload["type"] = this->type;

    if (transaction == Transaction::Request) payload["transaction"] = 0;
    else if (transaction == Transaction::Response) payload["transaction"] = 1;
    else payload["transaction"] = -1;

    if (transaction == Transaction::Response)
        payload["numberCore"] = this->_numberCore;

    return payload;
}
