//
// Created by ivan on 10.03.2026.
//

#include "../include/SignalMessage.hpp"

#include <string>


SignalMessage::SignalMessage(const std::string &type,const int central_Freq, std::vector<std::complex<float>> signal) :
    Message(type),
    _central_Freq(central_Freq), _signal(std::move(signal)) {}

SignalMessage::SignalMessage(const std::string &type, json::value &jv) : Message(type) {
    this->_central_Freq = jv.at("central_Freq").as_int64();
    auto& signalArray = jv.at("signal").as_array();

    for (auto const& value : signalArray){
        if (value.is_array() && value.as_array().size() == 2) {
            auto& complex_arr = value.as_array();
            float real = complex_arr[0].as_double();
            float imag = complex_arr[1].as_double();
            _signal.push_back(std::complex<float>(real, imag));
        }
    }
}


int SignalMessage::getCentralFreq() {
    return _central_Freq;
}

std::vector<std::complex<float>> SignalMessage::getSignal() {
    return _signal;
}

void SignalMessage::setTransactionType() {
    if (_signal.empty())
        this->transactionType = TransactionType::Request;
    else
        this->transactionType = TransactionType::Response;
}
