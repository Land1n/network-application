//
// Created by ivan on 07.03.2026.
//
#include "TransportHandler.hpp"

#include <iostream>

TransportHandler::TransportHandler(std::shared_ptr<tcp::socket> socket, uint32_t magicNumber, bool DEBUG)
    : socket(std::move(socket)), magicNumber(magicNumber) {
    if (DEBUG) logger.setLevel(LogLevel::Debug);
    else logger.setLevel(LogLevel::Error);
}

TransportMessage TransportHandler::read() {
    TransportMessage transport_message;
    boost::system::error_code error;
    uint32_t magic = 0;
    while (magic != magicNumber) {
        boost::asio::read(*socket, boost::asio::buffer(&magic, 4), error);
        if (error) {
            transport_message.type = "error";
            transport_message.transaction = Transaction::Error;
            return transport_message;
        }
    }
    uint32_t json_len;
    boost::asio::read(*socket, boost::asio::buffer(&json_len, 4), error);
    if (error) {
        transport_message.type = "error";
        transport_message.transaction = Transaction::Error;
        return transport_message;
    }
    transport_message.payload.resize(json_len);
    boost::asio::read(*socket, boost::asio::buffer(transport_message.payload), error);
    if (error) {
        transport_message.type = "error";
        transport_message.transaction = Transaction::Error;
        transport_message.payload.clear();
        return transport_message;
    }
    try {
        std::string_view json(reinterpret_cast<const char *>(transport_message.payload.data()),
                              transport_message.payload.size());
        json::value json_val = json::parse(json);
        if (onReadHandler) onReadHandler(0, transport_message.payload.data(), transport_message.payload.size());
        // пока не предумал как делать
        transport_message.type = json_val.at("type").as_string();
        transport_message.transaction = setTypeTransaction(json_val.at("transaction").as_int64());
        logger.log(LogLevel::Info, __func__, "Read request TransportMessage");
    } catch (const std::exception &e) {
        transport_message.type = "error";
        transport_message.transaction = Transaction::Error;
        logger.log(LogLevel::Critical, __func__, "Read request TransportMessage invalid msg");
        logger.log(LogLevel::Critical, __func__, e.what());
    }
    logger.log(LogLevel::Info, __func__, "Request TransportMessage.type: " + transport_message.type);
    return transport_message;
}

bool TransportHandler::write(TransportMessage &message) {
    uint32_t json_len = message.payload.size();
    logger.log(LogLevel::Info, __func__, "Response TransportMessage.type: " + message.type);

    if (onWriteHandler) onWriteHandler(message.payload);
    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(boost::asio::buffer(&magicNumber, 4));
    buffers.push_back(boost::asio::buffer(&json_len, 4));
    buffers.push_back(boost::asio::buffer(message.payload));
    boost::system::error_code error;
    try {
        boost::asio::write(*socket, buffers, error);
        if (!error) logger.log(LogLevel::Info, __func__, "Write response TransportMessage");
        else logger.log(LogLevel::Warn, __func__, "Write response TransportMessage failed");
    } catch (const std::exception &e) {
        logger.log(LogLevel::Critical, __func__, e.what());
    }
    return !error;
}

void TransportHandler::setOnReadHandler(std::function<void(size_t, const void *, size_t)> handler) {
    onReadHandler = handler;
}

void TransportHandler::setOnWriteHandler(std::function<void(std::vector<uint8_t> &)> handler) {
    onWriteHandler = handler;
}
