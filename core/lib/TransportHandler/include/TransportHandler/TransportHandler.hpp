//
// Created by ivan on 12.05.2026.
//
#pragma once

#include <boost/asio.hpp>
#include <boost/json.hpp>
#include "TransportMessage.hpp"
#include "Logger.hpp"

namespace json = boost::json;
using tcp      = boost::asio::ip::tcp;

enum class TypeTransportHandler {
	Sync = 0,
	Async = 1,
};

class TransportHandler {
public:
	TransportHandler(tcp::socket& socket, TypeTransportHandler type);

	void read(TransportMessage& in_message, boost::system::error_code& code);
	void write(const TransportMessage& out_message, boost::system::error_code& code);

	void setMagicNumber(uint32_t magicNumber);
	void setOnReadHandler(std::function<void(size_t id, const void*, size_t)> handler);
	void setOnWriteHandler(std::function<void(const void*, size_t)> handler);

protected:
	uint32_t magicNumber = 0xA0ABA0A;
	tcp::socket& socket;
	Logger& logger = Logger::getInstance();

	TypeTransportHandler type;

	std::function<void(size_t, const void*, size_t)> onReadHandler;
	std::function<void(const void*, size_t)> onWriteHandler;

	void sync_read(TransportMessage& in_message, boost::system::error_code& code);

	void async_read(TransportMessage& in_message, boost::system::error_code& code);

};
