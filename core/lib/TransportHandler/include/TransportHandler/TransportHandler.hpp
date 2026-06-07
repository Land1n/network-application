//
// Created by ivan on 12.05.2026.
//
#pragma once

#include <boost/asio.hpp>
#include <boost/json.hpp>
#include "TransportMessage.hpp"
#include "utils.hpp"
#include "IOContextHandler/IOContextHandler.hpp"

class TransportHandler {
public:
	TransportHandler(std::shared_ptr<tcp::socket>& socket);

	void read(IOMode mode);
	void write(TransportMessage&& out_message, IOMode mode);

	void setOnError(std::function<void(error_code)> handler);

	void setMagicNumber(uint32_t magicNumber);
	void setOnReadHandler(std::function<void(size_t id, const void*, size_t)> handler);
	void setOnWriteHandler(std::function<void(const void*, size_t)> handler);

	void setOnAllRead(const std::function<void(error_code, TransportMessage&&)>& handler);
	void setOnAllWrite(const std::function<void(error_code, TransportMessage&&)>& handler);

protected:
	static void setErrorMessage(TransportMessage& message);

	void read_sync();
	void read_async();

	uint32_t magicNumber = 0xA0ABA0A;
	std::shared_ptr<tcp::socket> socket;

	std::function<void(size_t, const void*, size_t)> onReadHandler;
	std::function<void(const void*, size_t)> onWriteHandler;

	std::function<void(error_code, TransportMessage&&)> onAllWrite;
	std::function<void(error_code, TransportMessage&&)> onAllRead;

	std::function<void(error_code)> onError;
};
