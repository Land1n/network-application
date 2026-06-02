//
// Created by guestuser on 29.05.2026.
//
#include "gtest/gtest.h"
#include <boost/asio.hpp>

#include "ConnectionHandler/ConnectionHandler.hpp"

class ConnectionHandlerTests : public ::testing::Test {
public:
	boost::asio::io_context context;
	std::unique_ptr<tcp::acceptor> acceptor;
	tcp::endpoint endpoint = tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), 8000);
	std::unique_ptr<tcp::socket> socket;

	std::shared_ptr<ConnectionHandler> ch;
	void SetUp() override
	{
		acceptor = std::make_unique<tcp::acceptor>(context);
		acceptor->open(endpoint.protocol());
		acceptor->set_option(tcp::acceptor::reuse_address(true));
		acceptor->bind(endpoint);
		socket = std::make_unique<tcp::socket>(context);

		ch = std::make_shared<ConnectionHandler>(*socket);
	}
	void TearDown() override
	{
		acceptor->cancel();
		acceptor->close();
		if(socket->is_open())
			socket->close();

		if (ch != nullptr) {
			ch->disconnect();
			ch.reset();
		}
	}

	void listenAcceptor()
	{
		acceptor->listen();
	}
	static void expect_success(error_code ec) { EXPECT_FALSE(ec); }
	static void expect_failure(error_code ec) { EXPECT_TRUE(ec); }
};
TEST_F(ConnectionHandlerTests, SyncConnectAndDisconnectSuccess)
{
	listenAcceptor();
	ch->setOnConnect(expect_success);
	ch->setOnDisconnect(expect_success);
	ch->connect("127.0.0.1", 8000, IOMode::Sync);
	ch->disconnect();
}

TEST_F(ConnectionHandlerTests, AsyncConnectAndDisconnectSuccess)
{
	listenAcceptor();
	ch->setOnDisconnect(expect_success);
	ch->setOnConnect([this](error_code error_code) {
		expect_success(error_code);
		ch->disconnect();

	});

	ch->connect("127.0.0.1", 8000,IOMode::Async);
	context.run();
}
TEST_F(ConnectionHandlerTests, SyncConnectAndDisconnectFailure)
{
	ch->setOnConnect(expect_failure);
	ch->connect("127.0.0.1", 8000,IOMode::Sync);
}

TEST_F(ConnectionHandlerTests, AsyncConnectAndDisconnectFailure)
{
	ch->setOnConnect(expect_failure);
	ch->connect("127.0.0.1", 8000,IOMode::Async);
	context.run();
}

TEST_F(ConnectionHandlerTests,AsyncConnectAndThrow)
{
	listenAcceptor();
	ch->setOnConnect([&](error_code ec) {
		expect_success(ec);
		throw std::runtime_error("test error");
	});
	ch->connect("127.0.0.1", 8000,IOMode::Async);
	context.run();

}