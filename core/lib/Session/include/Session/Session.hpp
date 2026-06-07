//
// Created by guestuser on 28.05.2026.
//
#pragma once
#include "ConnectionHandler/ConnectionHandler.hpp"
#include "TransportHandler/TransportHandler.hpp"
#include "MessageHandler.hpp"
#include "Message.hpp"
#include "AcceptHandler/AcceptHandler.hpp"

#include <string>
#include <boost/type.hpp>

enum class TypeSession {
	Sync  = 0,
	Async = 1,
};

class Session {
public:
	Session(boost::asio::io_context& io, IOMode type);
	~Session();

	void connect(const std::string& address, unsigned int port);
	void connect(const tcp::endpoint& endpoint);
	void accept(AcceptHandler& accept_handler);

	void write(const std::unique_ptr<Message>& message);
	void read();

	void disconnect();

	void setOnConnect(const CallBack&);
	void setOnAccept(const CallBack&);

	void setOnAllWrite(const CallBack&);
	void setOnAllRead(const std::function<void(error_code, std::unique_ptr<Message>&&)>&);

	void setOnError(const CallBack&);

protected:
	IOMode mode;

	std::unique_ptr<ConnectionHandler> connection_handler;
	std::unique_ptr<TransportHandler> transport_handler;
	MessageHandler message_handler;

	std::shared_ptr<tcp::socket> socket;

	CallBack on_connect;
	CallBack on_accept;

	CallBack on_all_write;
	std::function<void(error_code, std::unique_ptr<Message>&&)> on_all_read;
};