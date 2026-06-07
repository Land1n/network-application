//
// Created by guestuser on 01.06.2026.
//
#include <gtest/gtest.h>

#include <thread>
#include "AcceptHandler/AcceptHandler.hpp"
#include "ConnectionHandler/ConnectionHandler.hpp"

#include "IOContextHandler/IOContextHandler.hpp"

class AcceptConnectionHandlerTests : public ::testing::Test {
public:
	IOContextHandler io_server;
	IOContextHandler io_client;

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
};

inline void expect_success(error_code ec)
{
	EXPECT_FALSE(ec);
}
inline void expect_failure(error_code ec)
{
	EXPECT_TRUE(ec);
}
TEST_F(AcceptConnectionHandlerTests, AllSyncConnectAndAccept)
{
	ConnectionHandler connection_handler(socket_client);
	AcceptHandler accept_handler(io_server.getIOContext(), endpoint);
	connection_handler.setOnConnect(expect_success);
	connection_handler.setOnDisconnect(expect_success);
	accept_handler.setOnAccept(expect_success);

	std::thread t([&]() {
		accept_handler.accept(socket_server, IOMode::Sync);
	});
	connection_handler.connect(endpoint, IOMode::Sync);
	t.join();
}

TEST_F(AcceptConnectionHandlerTests, AllAsyncConnectAndAccept)
{
	ConnectionHandler connection_handler(socket_client);
	AcceptHandler accept_handler(io_server.getIOContext(), endpoint);

	std::promise<void> connect_done;
	std::promise<void> accept_done;
	connection_handler.setOnConnect([&](error_code ec) {
		expect_success(ec);
		connect_done.set_value();
	});
	accept_handler.setOnAccept([&](error_code ec) {
		expect_success(ec);
		accept_done.set_value();
	});

	connection_handler.connect(endpoint, IOMode::Async);
	accept_handler.accept(socket_server, IOMode::Async);

	connect_done.get_future().wait();
	accept_done.get_future().wait();
}