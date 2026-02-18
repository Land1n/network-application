#pragma once

#include <iostream>
#include <vector>


template<typename Format>
struct DataPacket {
    std::string type;
    unsigned int sizePacket;
    unsigned int dataLenght;
    std::vector<Format> data;
    DataPacket(const std::string& type, unsigned int sizePacket = 0, unsigned int dataLenght = 0)
        : type(type), sizePacket(sizePacket), dataLenght(dataLenght)
    {}

};