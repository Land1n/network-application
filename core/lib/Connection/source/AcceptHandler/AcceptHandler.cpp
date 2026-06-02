//
// Created by guestuser on 01.06.2026.
//
#include "AcceptHandler/AcceptHandler.hpp"

AcceptHandler::AcceptHandler(boost::asio::io_context& io, const std::string& address, unsigned int port) : acceptor(io)
{
	setOptionAcceptor(address, port);
}
AcceptHandler::AcceptHandler(boost::asio::io_context& io, const tcp::endpoint& endpoint) : acceptor(io)
{
	setOptionAcceptor(endpoint);
}

AcceptHandler::~AcceptHandler()
{
	close();
}

void AcceptHandler::setOnAccept(const CallBack& c)
{
	onAccept = c;
}
void AcceptHandler::setOnClose(const CallBack& c)
{
	onClose = c;
}

void AcceptHandler::setOptionAcceptor(const std::string& address, int port)
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

void AcceptHandler::accept(tcp::socket& socket, IOMode mode)
{
	if(mode == IOMode::Sync) {
		sync_accept(socket);
	}
	else {
		async_accept(socket);
	}
}
void AcceptHandler::sync_accept(tcp::socket& socket)
{
	error_code ec;
	acceptor.accept(socket, ec);
	if (onAccept)
		onAccept(ec);
	ErrorHandler::check_error(ec, "AcceptHandler::accept.sync");
}
void AcceptHandler::async_accept(tcp::socket& socket)
{
	auto self    = shared_from_this();
	acceptor.async_accept(socket, [this, self](error_code ec) {
		ErrorHandler::check_error(ec, "AcceptHandler::accept.async");
		if(onAccept)
			onAccept(ec);
	});
}
void AcceptHandler::close()
{
	error_code ec;
	std::promise<error_code> promise;
	if(acceptor.is_open()) {
		acceptor.cancel(ec);
		ErrorHandler::check_error(ec, "AcceptHandler::cancel");
		acceptor.close(ec);
		if (onClose) onClose(ec);
		ErrorHandler::check_error(ec, "AcceptHandler::close");
	}
}
