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
	void SetUp() override
	{
		acceptor = std::make_unique<tcp::acceptor>(context);
		acceptor->open(endpoint.protocol());
		acceptor->set_option(tcp::acceptor::reuse_address(true));
		acceptor->bind(endpoint);
		socket = std::make_unique<tcp::socket>(context);
	}
	void TearDown() override
	{
		acceptor->cancel();
		acceptor->close();
		if(socket->is_open())
			socket->close();
	}

	void listenAcceptor()
	{
		acceptor->listen();
	}

	bool check_error(tcp::socket& socket)
	{
		error_code ec;
		boost::asio::write(socket, boost::asio::buffer(std::string("1")), ec);
		return ec.value();
	}
};

TEST_F(ConnectionHandlerTests, SyncConnectAndDisconnectSuccess)
{
	listenAcceptor();

	auto ch = std::make_shared<ConnectionHandler>(*socket, TypeConnectionHandler::Sync);
	ch->connect("127.0.0.1", 8000);
	EXPECT_FALSE(check_error(*socket));
	ch->disconnect();
	EXPECT_TRUE(check_error(*socket));
}

TEST_F(ConnectionHandlerTests, AsyncConnectAndDisconnectSuccess)
{
	listenAcceptor();

	auto ch = std::make_shared<ConnectionHandler>(*socket, TypeConnectionHandler::Async);
	ch->setCallback([&](error_code ec) {
		EXPECT_FALSE(ec);
		EXPECT_FALSE(check_error(*socket));
		ch->disconnect();
		EXPECT_TRUE(check_error(*socket));
	});
	ch->connect("127.0.0.1", 8000);
	context.run();
}
TEST_F(ConnectionHandlerTests, SyncConnectAndDisconnectFailure)
{
	auto ch = std::make_shared<ConnectionHandler>(*socket, TypeConnectionHandler::Sync);
	ch->connect("127.0.0.1", 8000);
	EXPECT_TRUE(check_error(*socket));
	EXPECT_FALSE(socket->is_open());
}

TEST_F(ConnectionHandlerTests, AsyncConnectAndDisconnectFailure)
{
	auto ch = std::make_shared<ConnectionHandler>(*socket, TypeConnectionHandler::Async);
	ch->connect("127.0.0.1", 8000);
	context.run();
	EXPECT_TRUE(check_error(*socket));
	EXPECT_FALSE(socket->is_open());
}

TEST_F(ConnectionHandlerTests,AsyncConnectAndThrow)
{
	listenAcceptor();
	auto ch = std::make_shared<ConnectionHandler>(*socket, TypeConnectionHandler::Async);
	ch->setCallback([&](error_code ec) {
		throw std::runtime_error("test error");
	});
	ch->connect("127.0.0.1", 8000);
	context.run();
}