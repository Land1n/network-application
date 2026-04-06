//
// Created by ivan on 07.03.2026.
//

#include "TransportHandler.hpp"

#include <boost/json.hpp>

namespace json = boost::json;

TransportHandler::TransportHandler(std::shared_ptr<tcp::socket> socket, const uint32_t magicNumber,bool DEBUG)
    : socket(std::move(socket)), magicNumber(magicNumber) {
    logger.init5Levels();
    if (!DEBUG)
        logger.setLogLevel("ERROR");
    else
        logger.setLogLevel("DEBUG");
}


TransportMessage TransportHandler::read() {
    TransportMessage transport_message;
    boost::system::error_code error;

    uint32_t magic = 0;

    while (magic != magicNumber) {
        boost::asio::read(*socket, boost::asio::buffer(&magic, 4), error);
        if (error) return transport_message;
    }

    uint32_t json_len;
    boost::asio::read(*socket, boost::asio::buffer(&json_len, 4), error);
    if (error) return transport_message;

    transport_message.payload.resize(json_len);

    boost::asio::read(*socket, boost::asio::buffer(transport_message.payload), error);
    if (error) {
        transport_message.payload.clear();
    }
    try{
        std::string_view json(reinterpret_cast<const char*>(transport_message.payload.data()),transport_message.payload.size());
        json::value json_val = json::parse(json);
        transport_message.type = json_val.at("type").as_string();
        transport_message.transaction = setTypeTransaction(json_val.at("transaction").as_int64());
        logger("DEBUG") << "TransportHandler : Read request TransportMessage" << "\n";

    }  catch (std::exception &e) {
        transport_message.type = "error";
        transport_message.transaction = Transaction::Error;
        logger("WARN") << "TransportHandler : Read request TransportMessage" << "\n";
    }
    logger("DEBUG") << "TransportHandler : Request TransportMessage.type:" << transport_message.type << "\n";
    return transport_message;
}

bool TransportHandler::send(TransportMessage &message) {
    uint32_t json_len = message.payload.size();
    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(boost::asio::buffer(&magicNumber, 4));
    buffers.push_back(boost::asio::buffer(&json_len, 4));
    buffers.push_back(boost::asio::buffer(message.payload));

    boost::system::error_code error;
    boost::asio::write(*socket, buffers, error);
    if (!error)
        logger("DEBUG") << "TransportHandler : Send response TransportMessage" << "\n";
    else
        logger("WARN") << "TransportHandler : Send response TransportMessage" << "\n";
    return !error;
}
