#pragma once 

#include <iostream>

#include <boost/asio.hpp>
#include <vector>
#include <atomic>

#include "signal_data_packet.hpp" 

using boost::asio::ip::tcp;

class RequestResponseHandler
{
public:
    explicit RequestResponseHandler(std::shared_ptr<std::atomic<bool>> is_working,std::shared_ptr<tcp::socket> socket) 
        : is_working(is_working), socket(socket)
    {}
    void processingData();
    bool searchData(uint32_t magic_patern);
    SignalDataPacket parseData();
    void serializeData();
    void sendData();
    std::string getIp();
private:
    std::vector<char> data;
    std::shared_ptr<std::atomic<bool>> is_working;
    std::shared_ptr<tcp::socket> socket;
};
