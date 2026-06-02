//
// Created by guestuser on 01.06.2026.
//
#include "Session/Session.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

Session::Session(IOMode mode) : mode(mode)
{
	socket             = std::make_shared<tcp::socket>(io_context);
	connection_handler = std::make_shared<ConnectionHandler>(*socket);
	transport_handler  = std::make_shared<TransportHandler>(*socket);
	message_handler    = std::make_shared<MessageHandler>();

	if(mode == IOMode::Async) {
		startAsyncWork();
	}
}

void Session::startAsyncWork()
{
	work_io_context       = std::make_unique<boost::asio::io_context::work>(io_context);
	thread_for_io_context = std::thread([this]() {
		io_context.run();
	});
}

void Session::stopAsyncWork()
{
	if(work_io_context != nullptr) {
		work_io_context.reset();
		thread_for_io_context.join();
	}
}
Session::~Session()
{
	if(socket->is_open()) {
		disconnect();
	}
	if(mode == IOMode::Async) {
		stopAsyncWork();
	}
}

void Session::connect(const tcp::endpoint& endpoint)
{
	if(on_connect) {
		connection_handler->setOnConnect([this](error_code ec) {
			ErrorHandler::check_error(ec, std::string("Session::connect.") + getSyncOrAsync(mode), true);
			on_connect(ec);
		});
	}
	connection_handler->connect(endpoint, mode);
}
void Session::connect(const std::string& address, unsigned int port)
{
	auto endpoint = tcp::endpoint(boost::asio::ip::make_address(address), port);
	connect(endpoint);
}

void Session::disconnect()
{
	if(socket->is_open()) {
		connection_handler->setOnDisconnect([this](error_code ec) {
			ErrorHandler::check_error(ec, std::string("Session::disconnect.") + getSyncOrAsync(mode), true);
		});
		connection_handler->disconnect();
	}
}
void Session::setOnConnect(const CallBack& callback)
{
	on_connect = callback;
}

void Session::read(std::unique_ptr<Message>& message)
{
	TransportMessage transport_message;
	transport_handler->setOnAllRead([this, &message](error_code ec, TransportMessage transport_message) {
		if(!ErrorHandler::check_error(ec, std::string("Session::read.") + getSyncOrAsync(mode), true) &&
		   transport_message.transaction != Transaction::Error)
			message = message_handler->parse(transport_message);
		else
			disconnect();
	});
	transport_handler->read(transport_message, mode);
}

void Session::write(const std::unique_ptr<Message>& message)
{
	auto transport_message = message_handler->serialize(message);
	transport_handler->setOnAllWrite([this](error_code ec, TransportMessage) {
		if(ErrorHandler::check_error(ec, std::string("Session::write.") + getSyncOrAsync(mode), true))
			disconnect();
	});
	transport_handler->write(transport_message, mode);
}