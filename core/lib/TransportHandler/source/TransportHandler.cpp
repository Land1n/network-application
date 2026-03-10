//
// Created by ivan on 07.03.2026.
//

#include "TransportHandler.hpp"

TransportMessage TransportHandler::read() {
    TransportMessage message;
    boost::system::error_code error;

    uint32_t magic = 0;

    while (magic != magicNumber) {
        boost::asio::read(*socket, boost::asio::buffer(&magic, 4), error);
        if (error) return message;
    }

    uint32_t json_len;
    boost::asio::read(*socket, boost::asio::buffer(&json_len, 4), error);
    if (error) return message;

    message.payload.resize(json_len);

    boost::asio::read(*socket, boost::asio::buffer(message.payload), error);
    if (error) {
        message.payload.clear();
    }
    return message;
}

bool TransportHandler::send(TransportMessage &message) {
    uint32_t json_len = message.payload.size();
    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(boost::asio::buffer(&magicNumber, 4));
    buffers.push_back(boost::asio::buffer(&json_len, 4));
    buffers.push_back(boost::asio::buffer(message.payload));

    boost::system::error_code error;
    boost::asio::write(*socket, buffers, error);
    return !error;
}
