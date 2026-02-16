#pragma once 

#include <iostream>

#include <boost/asio.hpp>
#include <atomic>

using boost::asio::ip::tcp;

class RequestResponseHandler
{
public:
    explicit RequestResponseHandler(std::shared_ptr<std::atomic<bool>> is_working,std::shared_ptr<tcp::socket> socket) 
        : is_working(is_working), socket(socket)
    {}
    void processingData();

    void searchData();
    void serializeData();
    void sendData();
    std::string getIp();
private:
    char data[4096];
    std::shared_ptr<std::atomic<bool>> is_working;
    std::shared_ptr<tcp::socket> socket;
};
