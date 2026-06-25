//
// Created by guestuser on 05.06.2026.
//
#include "session/session_manager.h"
#include "session/session.h"
#include "io_context_handler/io_context_handler.h"
#include "message/information_message.h"

#include <gtest/gtest.h>

class SessionManagerTests : public ::testing::Test {
public:
	tcp::endpoint endpoint = tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), 8200);
	IOContextHandler client_context;
	IOContextHandler server_context;
};

TEST_F(SessionManagerTests, SyncErrorAndRemove)
{
	SessionManager sessionManager(server_context.getIOContext(), IOMode::Sync, endpoint);
	std::promise<void> connect_done;

	std::thread([&]() {
		sessionManager.addSession(0);
		connect_done.set_value();
	}).detach();

	auto session = std::make_shared<Session>(client_context.getIOContext(), IOMode::Sync);
	session->connect(endpoint);
	connect_done.get_future().wait();
	EXPECT_EQ(sessionManager.getSessionCount(), 1);
	session->disconnect();
	sessionManager.getSession(0)->read();
	EXPECT_EQ(sessionManager.getSessionCount(), 0);
}

TEST_F(SessionManagerTests, AsyncErrorAndRemove)
{
	SessionManager sessionManager(server_context.getIOContext(), IOMode::Async, endpoint);
	std::thread([&sessionManager]() {
		sessionManager.addSession(0);
	}).detach();

	Session session_client(client_context.getIOContext(), IOMode::Async);
	std::promise<void> connect_done;
	session_client.setOnConnect([&connect_done](error_code) {
		connect_done.set_value();
	});
	session_client.connect(endpoint);
	connect_done.get_future().wait();
	session_client.disconnect();

	auto session_on_server = sessionManager.getSession(0);
	std::promise<void> read_done;
	if(session_on_server) {
		session_on_server->setOnAllRead([&read_done, &sessionManager](error_code, std::unique_ptr<Message>&&) {
			EXPECT_EQ(sessionManager.getSessionCount(), 0);
			read_done.set_value();
		});
		session_on_server->read();
		read_done.get_future().wait();
	}
}