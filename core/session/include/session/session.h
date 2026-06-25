//
// Created by guestuser on 28.05.2026.
//
#pragma once

#include "connection/connect_handler.h"
#include "connection/accept_handler.h"
#include "transport_handler/transport_handler.h"
#include "message_handler/message_handler.h"
#include "message/message.h"
#include "clientserveriface/server.h"

#include <string>
#include <memory>
#include <functional>

enum class TypeSession {
	Sync  = 0,
	Async = 1,
};

class Session {
public:
	using ErrorAndMessageHandler = std::function<void(error_code, std::unique_ptr<Message>&&)>;

	Session(boost::asio::io_context& io, IOMode type);
	~Session();

	void connect(const std::string& address, PortInt port);
	void connect(const tcp::endpoint& endpoint);
	void accept(AcceptHandler& accept_handler);

	void write(const std::unique_ptr<Message>& message);
	void read();

	void disconnect();

	void setOnConnect(const CallbackError&);
	void setOnAccept(const CallbackError&);
	void setOnDisconnect(const CallbackError&);

	void setOnReadHandler(const Network::Server::ReadHandler&);

	void setOnAllWrite(const CallbackError&);
	void setOnAllRead(const ErrorAndMessageHandler&);

	void setOnError(const CallbackError&);

	void setMagicNumber(uint32_t magicNumber);

	bool hasError();

protected:
	IOMode mode;
	bool criticalError{false};

	std::unique_ptr<ConnectHandler> connection_handler;
	std::unique_ptr<TransportHandler> transport_handler;
	MessageHandler message_handler;

	std::shared_ptr<tcp::socket> socket;

	CallbackError on_connect;
	CallbackError on_accept;
	CallbackError on_disconnect;

	CallbackError on_all_write;
	ErrorAndMessageHandler on_all_read;
};