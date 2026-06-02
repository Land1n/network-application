//
// Created by guestuser on 28.05.2026.
//
#pragma once
#include "ConnectionHandler/ConnectionHandler.hpp"
#include "TransportHandler/TransportHandler.hpp"
#include "MessageHandler.hpp"
#include  "Message.hpp"

#include <string>
#include <boost/type.hpp>

enum class TypeSession {
	Sync = 0,
	Async = 1,
};

class Session : public std::enable_shared_from_this<Session> {
public:
	Session(IOMode type);
	~Session();
	void connect(const std::string& address, unsigned int port);
	void connect(const tcp::endpoint& endpoint);

	void write(const std::unique_ptr<Message>& message);
	void read(std::unique_ptr<Message>& message);

	void disconnect();

	void setOnConnect(const CallBack&);

	void startAsyncWork();
	void stopAsyncWork();

	bool hasError();
protected:

	IOMode mode;

	std::shared_ptr<ConnectionHandler> connection_handler;
	std::shared_ptr<TransportHandler> transport_handler;
	std::shared_ptr<MessageHandler> message_handler;

	boost::asio::io_context io_context;
	std::shared_ptr<tcp::socket> socket;

	CallBack on_connect;

	bool is_error = false;

	std::thread thread_for_io_context;
	std::unique_ptr<boost::asio::io_context::work> work_io_context;
};