//
// Created by guestuser on 01.06.2026.
//
#include <gtest/gtest.h>

#include <thread>
#include "AcceptHandler/AcceptHandler.hpp"
#include "ConnectionHandler/ConnectionHandler.hpp"


class AcceptConnectionHandlerTests : public ::testing::Test {
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

};

inline void expect_success(error_code ec) { EXPECT_FALSE(ec); }
inline void expect_failure(error_code ec) { EXPECT_TRUE(ec); }
TEST_F(AcceptConnectionHandlerTests, AllSyncConnectAndAccept)
{
	ConnectionHandler connection_handler(*socket_client);
	AcceptHandler accept_handler(io_server,endpoint);
	connection_handler.setOnConnect(expect_success);
	connection_handler.setOnDisconnect(expect_success);
	accept_handler.setOnAccept(expect_success);

	std::thread t([&]() {
		accept_handler.accept(*socket_server,IOMode::Sync);
	});
	connection_handler.connect(endpoint,IOMode::Sync);
	t.join();
}

TEST_F(AcceptConnectionHandlerTests, AllAsyncConnectAndAccept)
{
	auto connection_handler = std::make_shared<ConnectionHandler>(*socket_client);
	auto accept_handler = std::make_shared<AcceptHandler>(io_server,endpoint);

	connection_handler->setOnConnect(expect_success);
	connection_handler->setOnDisconnect(expect_success);
	accept_handler->setOnAccept(expect_success);

	connection_handler->connect(endpoint,IOMode::Async);
	io_client.run();
	accept_handler->accept(*socket_server,IOMode::Async);
	io_server.run();
}