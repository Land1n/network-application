//
// Created by ivan on 10.03.2026.
//

#pragma once

#include <complex>
#include <vector>

#include "Message.hpp"

#include "boost/json.hpp"

namespace json = boost::json;

class SignalMessage : public Message {
public:
    SignalMessage(const std::string &type,Transaction transaction,const int central_Freq, std::vector<std::complex<float>> signal);
    SignalMessage(const std::string &type,Transaction transaction,json::value &jv);
    int getCentralFreq();
    std::vector<std::complex<float>> getSignal();

    json::object serialize() override;


private:
    int _central_Freq;
    std::vector<std::complex<float>> _signal;
};