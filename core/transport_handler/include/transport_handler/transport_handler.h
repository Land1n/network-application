//
// Created by ivan on 12.05.2026.
//
#pragma once

#include "../../../../clientserveriface/include/clientserveriface/connectionid.h"

#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <memory>
#include <functional>
#include <cstdint>
#include "utils/alias.h"
#include "message/transport_message.h"
#include "io_context_handler/io_context_handler.h"

#include "clientserveriface/server.h"

#include "utils/alias.h"

class TransportHandler {
public:
	using ErrorAndMessageHandler = std::function<void(error_code, TransportMessage&&)>;
	using WriteHandler           = std::function<void(const void*, size_t)>;

	explicit TransportHandler(const std::shared_ptr<tcp::socket>& socket);

	void read(IOMode mode);
	void write(TransportMessage&& out_message, IOMode mode);

	void setOnError(CallbackError handler);

	void setMagicNumber(MagicInt magicNumber);
	void setOnReadHandler(const Network::Server::ReadHandler& handler);
	void setOnWriteHandler(const WriteHandler& handler);

	void setOnAllRead(const ErrorAndMessageHandler& handler);
	void setOnAllWrite(const ErrorAndMessageHandler& handler);

protected:
	static void setErrorMessage(TransportMessage& message);

	void read_sync();
	void read_async();

	uint32_t magicNumber;
	std::shared_ptr<tcp::socket> socket;

	/// + TODO: надо использовать alias
	Network::Server::ReadHandler onReadHandler;
	WriteHandler onWriteHandler;

	ErrorAndMessageHandler onAllWrite;
	ErrorAndMessageHandler onAllRead;

	CallbackError onError;
};