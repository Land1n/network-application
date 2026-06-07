//
// Created by guestuser on 01.06.2026.
//
#include "Session/Session.hpp"

#include <gtest/gtest.h>

#include "AcceptHandler/AcceptHandler.hpp"

#include "IOContextHandler/IOContextHandler.hpp"

#include "InformationMessage.hpp"

class SessionTests : public ::testing::Test {
public:
	tcp::endpoint endpoint = tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), 8100);
	IOContextHandler io_client;
	IOContextHandler io_server;

	static void expect_success(error_code ec)
	{
		EXPECT_FALSE(ec);
	}
	static void expect_failure(error_code ec)
	{
		EXPECT_TRUE(ec);
	}

	TransportMessage create_info_message(int id)
	{
		auto message = std::make_unique<InformationMessage>("information", Transaction::Response, id);
		MessageHandler handler;
		return handler.serialize(std::move(message));
	};
	int get_data_from_info_message(TransportMessage&& transportMessage)
	{
		MessageHandler handler;
		auto message      = handler.parse(std::move(transportMessage));
		auto info_message = dynamic_cast<InformationMessage*>(message.get());
		return info_message->getNumberCore();
	}
};

TEST_F(SessionTests, SyncTwoSessionReadAndWrite)
{
	Session client_session(io_client.getIOContext(), IOMode::Sync);
	Session server_session(io_server.getIOContext(), IOMode::Sync);
	AcceptHandler acceptHandler(io_server.getIOContext(), endpoint);
	client_session.setOnConnect([&](error_code ec) {
		EXPECT_FALSE(ec);
		client_session.write(std::make_unique<InformationMessage>("information", Transaction::Response, 100));
	});
	server_session.setOnAllRead([](error_code ec, std::unique_ptr<Message>&& message) {
		auto info = dynamic_cast<InformationMessage*>(message.get());
		EXPECT_EQ(info->getNumberCore(), 100);
	});

	std::thread t_accept([&]() {
		server_session.setOnAccept([&](error_code ec) {
			EXPECT_FALSE(ec);
			server_session.read();
		});
		server_session.accept(acceptHandler);
	});
	client_session.connect(endpoint);
	t_accept.join();
}

TEST_F(SessionTests, AsyncTwoSessionReadAndWrite)
{
	Session client_session(io_client.getIOContext(), IOMode::Async);
	Session server_session(io_server.getIOContext(), IOMode::Async);
	AcceptHandler acceptHandler(io_server.getIOContext(), endpoint);

	std::promise<void> read_done;

	client_session.setOnConnect([&client_session](error_code ec) {
		EXPECT_FALSE(ec);
		client_session.write(std::make_unique<InformationMessage>("information", Transaction::Response, 100));
	});
	server_session.setOnAllRead([&read_done](error_code ec, std::unique_ptr<Message>&& message) {
		EXPECT_NE(message, nullptr);
		auto info = dynamic_cast<InformationMessage*>(message.get());
		EXPECT_EQ(info->getNumberCore(), 100);
		read_done.set_value();
	});

	server_session.setOnAccept([&server_session](error_code ec) {
		EXPECT_FALSE(ec);
		server_session.read();
	});

	server_session.accept(acceptHandler);
	client_session.connect(endpoint);

	read_done.get_future().wait();
}