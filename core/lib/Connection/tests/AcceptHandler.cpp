//
// Created by guestuser on 01.06.2026.
//
#include "AcceptHandler/AcceptHandler.hpp"
#include <thread>
#include <gtest/gtest.h>

class AcceptHandlerTests : public ::testing::Test {
public:
	boost::asio::io_context io_server;
	boost::asio::io_context io_client;

	std::unique_ptr<tcp::socket> socket_client;
	std::unique_ptr<tcp::socket> socket_server;

	tcp::endpoint endpoint = tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), 8000);

	void SetUp() override
	{
		socket_client = std::make_unique<tcp::socket>(io_client);
		socket_server = std::make_unique<tcp::socket>(io_server);
	}
	void TearDown() override
	{
		if(socket_server->is_open()) {
			socket_server->close();
			socket_server.reset();
		}
		if(socket_client->is_open()) {
			socket_client->close();
			socket_client.reset();
		}
	}
	static void expect_success(error_code ec) { EXPECT_FALSE(ec); }
	static void expect_failure(error_code ec) { EXPECT_TRUE(ec); }
};

TEST_F(AcceptHandlerTests, SyncAcceptSuccesful)
{
	AcceptHandler acceptor(io_server, endpoint);
	std::thread t([&] {
		acceptor.accept(*socket_server,IOMode::Sync);
		acceptor.setOnAccept(expect_success);
	});
	error_code ec;
	socket_client->connect(endpoint, ec);
	EXPECT_FALSE(ec);
	t.join();
}

TEST_F(AcceptHandlerTests, AsyncAcceptSuccesful)
{
	auto acceptor = std::make_shared<AcceptHandler>(io_server, endpoint);
	acceptor->setOnAccept([](error_code ec) {
		EXPECT_FALSE(ec);
	});
	acceptor->accept(*socket_server,IOMode::Async);
	std::thread t([&] {
		io_server.run();
	});
	error_code ec1;
	socket_client->connect(endpoint, ec1);
	EXPECT_FALSE(ec1);
	t.join();
}
