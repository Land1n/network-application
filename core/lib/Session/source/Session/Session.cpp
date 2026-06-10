//
// Created by guestuser on 01.06.2026.
//
#include "Session/Session.hpp"

#include "AcceptHandler/AcceptHandler.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

std::string getSyncOrAsync(IOMode mode)
{
	return (mode == IOMode::Sync ? "sync" : "async");
}

Session::Session(boost::asio::io_context& io, IOMode mode) : mode(mode)
{
	socket             = std::make_shared<tcp::socket>(io);
	connection_handler = std::make_unique<ConnectionHandler>(socket);
	transport_handler  = std::make_unique<TransportHandler>(socket);
}

Session::~Session()
{
	error_code error;
	disconnect();
	ErrorHandler::check_error(error, "Session::~Session");
}

void Session::connect(const tcp::endpoint& endpoint)
{
	connection_handler->setOnConnect([this](error_code ec) {
		ErrorHandler::check_error(ec, std::string("Session::connect.") + getSyncOrAsync(mode), true);
		if(on_connect)
			on_connect(ec);
	});
	connection_handler->connect(endpoint, mode);
}

void Session::accept(AcceptHandler& accept_handler)
{
	accept_handler.setOnAccept([this](error_code ec) {
		ErrorHandler::check_error(ec, std::string("Session::accept.") + getSyncOrAsync(mode), true);
		if(on_accept)
			on_accept(ec);
	});
	accept_handler.accept(socket, mode);
}

void Session::connect(const std::string& address, unsigned int port)
{
	auto endpoint = tcp::endpoint(boost::asio::ip::make_address(address), port);
	connect(endpoint);
}

void Session::disconnect()
{
	connection_handler->setOnDisconnect([this](error_code ec) {
		ErrorHandler::check_error(ec, std::string("Session::disconnect"), true);
		if(on_disconnect)
			on_disconnect(ec);
	});
	connection_handler->disconnect();
}
void Session::setOnConnect(const CallBack& callback)
{
	on_connect = callback;
}

void Session::setOnAccept(const CallBack& callback)
{
	on_accept = callback;
}
void Session::setOnDisconnect(const CallBack& callback)
{
	on_disconnect = callback;
}
void Session::setOnReadHandler(std::function<void(size_t, const void*, size_t)> handler)
{
	transport_handler->setOnReadHandler(handler);
}
void Session::setOnAllRead(const std::function<void(error_code, std::unique_ptr<Message>&&)>& callback)
{
	on_all_read = callback;
}
void Session::setOnError(const CallBack& callback)
{
	transport_handler->setOnError(callback);
	connection_handler->setOnError(callback);
}

void Session::setOnAllWrite(const CallBack& callback)
{
	on_all_write = callback;
}

void Session::read()
{
	transport_handler->setOnAllRead([this](error_code ec, TransportMessage&& transportMessage) {
		if(!ErrorHandler::check_error(ec, std::string("Session::read.") + getSyncOrAsync(mode), true)) {
			auto msg = message_handler.parse(transportMessage);
			if(on_all_read) {
				if(msg == nullptr) {
					ErrorHandler::check_error(ec, std::string("Session::read.") + getSyncOrAsync(mode) +
					                                  "{Message=nullptr}");
				}
				on_all_read(ec, std::move(msg));
			}
		}
		else {
			if(on_all_read) {
				ErrorHandler::check_error(ec,
				                          std::string("Session::read.") + getSyncOrAsync(mode) + "{Message=nullptr}");
				on_all_read(ec, nullptr);
			}
			disconnect();
		}
	});
	if(socket->is_open())
		transport_handler->read(mode);
}

void Session::write(const std::unique_ptr<Message>& message)
{
	auto transport_message = message_handler.serialize(message);
	transport_handler->setOnAllWrite([this](error_code ec, TransportMessage&& transportMessage) {
		if(!ErrorHandler::check_error(ec, std::string("Session::write.") + getSyncOrAsync(mode), true)) {
			if(on_all_write)
				on_all_write(ec);
		}
		else {
			if(on_all_write)
				on_all_write(ec);
			disconnect();
		}
	});
	transport_handler->write(std::move(transport_message), mode);
}