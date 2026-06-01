//
// Created by guestuser on 01.06.2026.
//
#include "AcceptHandler/AcceptHandler.hpp"

AcceptHandler::AcceptHandler(boost::asio::io_context& io, const std::string& address,unsigned int port,
                             TypeAcceptHandler type) :
    acceptor(io), type(type)
{
	setOptionAcceptor(address, port);
}
AcceptHandler::AcceptHandler(boost::asio::io_context& io, const tcp::endpoint& endpoint, TypeAcceptHandler type) :
	acceptor(io), type(type)
{
	setOptionAcceptor(endpoint);
}


AcceptHandler::~AcceptHandler()
{
	close();
}

void AcceptHandler::setCallback(const AsyncCallBack& c)
{
	callback = c;
};

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

void AcceptHandler::accept(tcp::socket& socket)
{
	if(type == TypeAcceptHandler::Sync) {
		sync_accept(socket);
	}
	else if(type == TypeAcceptHandler::Async) {
		async_accept(socket);
	}
}
void AcceptHandler::sync_accept(tcp::socket& socket)
{
	error_code ec;
	acceptor.accept(socket, ec);
	ErrorHandler::check_error(ec, "AcceptHandler::accept.sync");
}
void AcceptHandler::async_accept(tcp::socket& socket)
{
	auto self = shared_from_this();
	acceptor.async_accept(socket, [this, self, &socket](error_code ec) {
		ErrorHandler::check_error(ec, "AcceptHandler::accept.async");
		if(callback) {
			callback(ec);
			ErrorHandler::check_error(ec, "AcceptHandler::accept.async{Callback}");
		}
	});
}

void AcceptHandler::close()
{
	boost::system::error_code ec;
	if(type == TypeAcceptHandler::Async) {
		acceptor.cancel(ec);
		ErrorHandler::check_error(ec, "AcceptHandler::cancel");
	}
	if(acceptor.is_open()) {
		acceptor.close(ec);
		ErrorHandler::check_error(ec, "AcceptHandler::close");
	}
}
