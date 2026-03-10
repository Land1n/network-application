//
// Created by ivan on 07.03.2026.
//

#pragma once

#include "boost/asio.hpp"

#include "TransportMessage.hpp"

using tcp = boost::asio::ip::tcp;

class TransportHandler {
public:
    explicit TransportHandler(std::shared_ptr<tcp::socket> socket, const uint32_t magicNumber = 0xA0ABA0A) : socket(std::move(socket)), magicNumber(magicNumber) {}

    TransportMessage read();
    bool send(TransportMessage &message);
private:
    uint32_t magicNumber;
    std::shared_ptr<tcp::socket> socket;
};