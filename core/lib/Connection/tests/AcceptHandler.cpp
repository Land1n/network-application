//
// Created by guestuser on 01.06.2026.
//
#include "AcceptHandler/AcceptHandler.hpp"

#include "IOContextHandler/IOContextHandler.hpp"

#include <thread>
#include <gtest/gtest.h>

class AcceptHandlerTests : public ::testing::Test {
public:
	IOContextHandler io_client;
	IOContextHandler io_server;

	std::shared_ptr<tcp::socket> socket_client;
	std::shared_ptr<tcp::socket> socket_server;

	tcp::endpoint endpoint = tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), 8000);

	void SetUp() override
	{
		socket_client = std::make_shared<tcp::socket>(io_client.getIOContext());
		socket_server = std::make_shared<tcp::socket>(io_server.getIOContext());
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
	static void expect_success(error_code ec)
	{
		EXPECT_FALSE(ec);
	}
	static void expect_failure(error_code ec)
	{
		EXPECT_TRUE(ec);
	}
};

TEST_F(AcceptHandlerTests, SyncAcceptSuccesful)
{
	AcceptHandler acceptor(io_server.getIOContext(), endpoint);
	std::thread t([&] {
		acceptor.accept(socket_server, IOMode::Sync);
		acceptor.setOnAccept(expect_success);
	});
	error_code ec;
	socket_client->connect(endpoint, ec);
	EXPECT_FALSE(ec);
	t.join();
}

TEST_F(AcceptHandlerTests, AsyncAcceptSuccesful)
{
	AcceptHandler acceptor(io_server.getIOContext(), endpoint);
	std::promise<void> promise;
	acceptor.setOnAccept([&promise](error_code ec) {
		EXPECT_FALSE(ec);
		promise.set_value();
	});
	acceptor.accept(socket_server, IOMode::Async);
	error_code ec1;
	socket_client->connect(endpoint, ec1);
	promise.get_future().wait();
	EXPECT_FALSE(ec1);
}
