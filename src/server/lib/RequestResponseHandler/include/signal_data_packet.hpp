#pragma once

#include <boost/json.hpp>

#include <complex>
#include "data_packet.hpp"

namespace json = boost::json;

struct SignalDataPacket : DataPacket<std::complex<float>> {
    unsigned int centralFrequency;  
    SignalDataPacket(unsigned int sizePacket = 0, unsigned int dataLenght = 0,unsigned int centralFrequency = 0)
        : DataPacket<std::complex<float>>("signal",sizePacket,dataLenght),
          centralFrequency(centralFrequency)
    {}
    void setData(json::array& arr);
};
