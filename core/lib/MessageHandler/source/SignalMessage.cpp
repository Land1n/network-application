//
// Created by ivan on 10.03.2026.
//

#include "SignalMessage.hpp"

#include <string>


SignalMessage::SignalMessage(const std::string &type,Transaction transaction,const int central_Freq, std::vector<std::complex<float>> signal) :
    Message(type,transaction),
    _central_Freq(central_Freq), _signal(std::move(signal)) {}

SignalMessage::SignalMessage(const std::string &type,Transaction transaction, json::value &jv) : Message(type,transaction) {
    if (transaction == Transaction::Response) {
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
}


int SignalMessage::getCentralFreq() {
    return _central_Freq;
}

std::vector<std::complex<float>> SignalMessage::getSignal() {
    return _signal;
}

json::object SignalMessage::serialize() {
    json::object payload;
    payload["type"] = type;

    if (transaction == Transaction::Request) payload["transaction"] = 0;
    else if (transaction == Transaction::Response) payload["transaction"] = 1;
    else payload["transaction"] = -1;

    payload["central_Freq"] = _central_Freq;
    if (transaction == Transaction::Response) {
        json::array signal;
        for (auto const& c : _signal) {
                signal.push_back({c.real(), c.imag()});
        }
        payload["signal"] = signal;
    }
    return payload;
}
