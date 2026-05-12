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

class BaseTransportHandler {
public:
	explicit BaseTransportHandler(const std::shared_ptr<tcp::socket>& socket, uint32_t magicNumber);

	virtual void read(TransportMessage& in_message, boost::system::error_code& code)         = 0;
	virtual void write(const TransportMessage& out_message, boost::system::error_code& code) = 0;

	void setOnReadHandler(std::function<void(size_t id, const void*, size_t)> handler);
	void setOnWriteHandler(std::function<void(const void*, size_t)> handler);

	virtual ~BaseTransportHandler() = default;

protected:
	uint32_t magicNumber;
	std::shared_ptr<tcp::socket> socket;
	Logger& logger = Logger::getInstance();
	std::function<void(size_t, const void*, size_t)> onReadHandler;
	std::function<void(const void*, size_t)> onWriteHandler;
};
