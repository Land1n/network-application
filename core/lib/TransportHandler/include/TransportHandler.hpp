//
// Created by ivan on 07.03.2026.
//
#pragma once

#include <boost/asio.hpp>
#include <boost/json.hpp>
#include "TransportMessage.hpp"
#include "Logger.hpp"

namespace json = boost::json;
using tcp = boost::asio::ip::tcp;

class TransportHandler {
public:
    explicit TransportHandler(std::shared_ptr<tcp::socket> socket, uint32_t magicNumber = 0xA0ABA0A, bool DEBUG = false);
    TransportMessage read();
    bool write(TransportMessage &message);

    void setOnReadHandler(std::function<void(size_t id,const void *, size_t)> handler);
    void setOnWriteHandler(std::function<void(std::vector<uint8_t>&)> handler);
private:
    uint32_t magicNumber;
    std::shared_ptr<tcp::socket> socket;
    Logger& logger = Logger::getInstance();
    std::function<void(size_t,const void*, size_t)> onReadHandler;
    std::function<void(std::vector<uint8_t>&)> onWriteHandler;
};