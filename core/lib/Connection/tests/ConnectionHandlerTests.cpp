//
// Created by guestuser on 29.05.2026.
//
#include "gtest/gtest.h"
#include <boost/asio.hpp>

#include "ConnectionHandler/ConnectionHandler.hpp"
#include "IOContextHandler/IOContextHandler.hpp"

class ConnectionHandlerTests : public ::testing::Test {
public:
	IOContextHandler context_acceptor;
	IOContextHandler context_socket;
	std::unique_ptr<tcp::acceptor> acceptor;
	tcp::endpoint endpoint = tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), 8000);
	std::shared_ptr<tcp::socket> socket;

	void SetUp() override
	{
		acceptor = std::make_unique<tcp::acceptor>(context_acceptor.getIOContext());
		acceptor->open(endpoint.protocol());
		acceptor->set_option(tcp::acceptor::reuse_address(true));
		acceptor->bind(endpoint);
		socket = std::make_shared<tcp::socket>(context_socket.getIOContext());
	}
	void TearDown() override
	{
		acceptor->cancel();
		acceptor->close();
	}

	void listenAcceptor()
	{
		acceptor->listen();
	}
	static void expect_success(error_code ec)
	{
		EXPECT_FALSE(ec);
	}
	static void expect_failure(error_code ec)
	{
		EXPECT_TRUE(ec);
	}
};
TEST_F(ConnectionHandlerTests, SyncConnectAndDisconnectSuccess)
{
	listenAcceptor();
	ConnectionHandler ch(socket);
	ch.setOnConnect(expect_success);
	ch.setOnDisconnect(expect_success);
	ch.connect("127.0.0.1", 8000, IOMode::Sync);
	ch.disconnect();
}

TEST_F(ConnectionHandlerTests, AsyncConnectAndDisconnectSuccess)
{
	listenAcceptor();
	ConnectionHandler ch(socket);
	std::promise<void> do_disconnect;
	ch.setOnDisconnect([&do_disconnect](error_code ec) {
		EXPECT_FALSE(ec);
		do_disconnect.set_value();
	});
	ch.setOnConnect([&ch](error_code ec) {
		EXPECT_FALSE(ec);
		ch.disconnect();
	});

	ch.connect("127.0.0.1", 8000, IOMode::Async);
	do_disconnect.get_future().wait();
}

TEST_F(ConnectionHandlerTests, AsyncConnectAndThrow)
{
	listenAcceptor();
	std::promise<void> do_connect;
	ConnectionHandler ch(socket);

	ch.setOnConnect([&](error_code ec) {
		EXPECT_FALSE(ec);
		do_connect.set_value();
		throw std::runtime_error("test error");
	});
	ch.connect("127.0.0.1", 8000, IOMode::Async);
	do_connect.get_future().wait();
}