//
// Created by guestuser on 01.06.2026.
//
#include "connection/accept_handler.h"

#include "logger/logger.h"
AcceptHandler::AcceptHandler(boost::asio::io_context& io, const std::string& address, PortInt port) : acceptor(io)
{
	setOptionAcceptor(address, port);
}
AcceptHandler::AcceptHandler(boost::asio::io_context& io, PortInt port) : acceptor(io)
{
	setOptionAcceptor(port);
}
AcceptHandler::AcceptHandler(boost::asio::io_context& io, const tcp::endpoint& endpoint) : acceptor(io)
{
	setOptionAcceptor(endpoint);
}

AcceptHandler::~AcceptHandler()
{
	if(!acceptor.is_open())
		return;
	error_code ec;
	acceptor.cancel(ec);
	ErrorHandler::check_error(ec, "AcceptHandler::~AcceptHandler{cancel}");
	acceptor.close(ec);
	ErrorHandler::check_error(ec, "AcceptHandler::~AcceptHandler{close}");
}

void AcceptHandler::setOnAccept(const CallbackError& c)
{
	onAccept = c;
}
void AcceptHandler::setOnClose(const CallbackError& c)
{
	onClose = c;
}
void AcceptHandler::setOnError(const CallbackError& c)
{
	onError = c;
}
void AcceptHandler::getIOContext()
{}
void AcceptHandler::setOptionAcceptor(PortInt port)
{
	acceptor.open(tcp::v4());
	boost::system::error_code ec;
	acceptor.set_option(tcp::acceptor::reuse_address(true));
	acceptor.bind(tcp::endpoint(tcp::v4(), port), ec);
	ErrorHandler::check_error(ec, "AcceptHandler::setOptionAcceptor");
	acceptor.listen();
};

void AcceptHandler::setOptionAcceptor(const std::string& address, PortInt port)
{
	setOptionAcceptor(tcp::endpoint(boost::asio::ip::make_address(address), port));
};

void AcceptHandler::setOptionAcceptor(const tcp::endpoint& endpoint)
{
	acceptor.open(tcp::v4());
	boost::system::error_code ec;
	acceptor.set_option(tcp::acceptor::reuse_address(true));
	acceptor.bind(endpoint, ec);
	ErrorHandler::check_error(ec, "AcceptHandler::setOptionAcceptor");
	acceptor.listen();
}

void AcceptHandler::accept(std::shared_ptr<tcp::socket> socket, IOMode mode)
{
	if(mode == IOMode::Sync) {
		sync_accept(socket);
	}
	else {
		async_accept(socket);
	}
}
void AcceptHandler::sync_accept(std::shared_ptr<tcp::socket>& socket)
{
	std::promise<error_code> promise;
	acceptor.async_accept(*socket, [&promise](error_code ec) {
		promise.set_value(ec);
	});
	error_code ec = promise.get_future().get();
	if(ErrorHandler::check_error(ec, "AcceptHandler::accept.sync")) {
		if(onError)
			onError(ec);
	}
	if(onAccept)
		onAccept(ec);
}
void AcceptHandler::async_accept(std::shared_ptr<tcp::socket>& socket)
{
	acceptor.async_accept(*socket, [this](error_code ec) {
		if(ErrorHandler::check_error(ec, "AcceptHandler::accept.async"))
			if(onError)
				onError(ec);
		if(onAccept)
			onAccept(ec);
	});
}
void AcceptHandler::close()
{
	error_code ec;
	if(acceptor.is_open()) {
		acceptor.cancel(ec);
		ErrorHandler::check_error(ec, "AcceptHandler::cancel");
		acceptor.close(ec);
		if(onClose)
			onClose(ec);
		ErrorHandler::check_error(ec, "AcceptHandler::close");
	}
}
