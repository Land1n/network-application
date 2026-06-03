//
// Created by ivan on 12.05.2026.
//
#pragma once

#include <boost/asio.hpp>
#include <boost/json.hpp>
#include "TransportMessage.hpp"
#include "utils.hpp"


class TransportHandler {
public:
	TransportHandler(tcp::socket& socket);

	void read(TransportMessage& in_message, IOMode mode);
	void write(const TransportMessage& out_message, IOMode mode);

	void setOnError(std::function<void()> handler);

	void setMagicNumber(uint32_t magicNumber);
	void setOnReadHandler(std::function<void(size_t id, const void*, size_t)> handler);
	void setOnWriteHandler(std::function<void(const void*, size_t)> handler);

	void setOnAllRead(const std::function<void(error_code,TransportMessage)>& handler);
	void setOnAllWrite(const std::function<void(error_code,TransportMessage)>& handler);

protected:
	uint32_t magicNumber = 0xA0ABA0A;
	tcp::socket& socket;

	std::function<void(size_t, const void*, size_t)> onReadHandler;
	std::function<void(const void*, size_t)> onWriteHandler;

	std::function<void(error_code,TransportMessage)> onAllWrite;
	std::function<void(error_code,TransportMessage)> onAllRead;

	std::function<void()> onError;
};
