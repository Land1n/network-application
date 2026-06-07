//
// Created by guestuser on 05.06.2026.
//
#include "InformationMessage.hpp"
#include "SessionManager/SessionManager.hpp"

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
	std::thread([&]() {
		sessionManager.addSession(0);
	}).detach();
	auto session = std::make_shared<Session>(client_context.getIOContext(), IOMode::Sync);
	session->connect(endpoint);
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
	session_client.connect(endpoint);
	EXPECT_EQ(sessionManager.getSessionCount(), 1);
	session_client.disconnect();

	auto session_on_server = sessionManager.getSession(0);
	std::promise<void> read_done;
	session_on_server->setOnAllRead([&read_done, &sessionManager](error_code, std::unique_ptr<Message>&&) {
		EXPECT_EQ(sessionManager.getSessionCount(), 0);
		read_done.set_value();
	});
	session_on_server->read();
	read_done.get_future().wait();
}

// Тест на имитацию 3 подключений
TEST_F(SessionManagerTests, SyncSessionManagerWith3Session)
{
	SessionManager sessionManager(server_context.getIOContext(), IOMode::Sync, endpoint);

	std::thread server(
	    [&sessionManager]() { // Почему поток создается в потоке? Потому что для каждого пользователя свой поток
		    std::thread([&]() {
			    sessionManager.addSession(0);
			    auto session_on_server = sessionManager.getSession(0);
			    session_on_server->write(std::make_unique<InformationMessage>("information", Transaction::Response, 1));
		    }).detach();

		    std::thread([&]() {
			    sessionManager.addSession(2);
			    auto session_on_server = sessionManager.getSession(2);
			    session_on_server->setOnAllRead([](error_code ec, std::unique_ptr<Message>&& msg) {
				    EXPECT_FALSE(ec);
				    auto info = dynamic_cast<InformationMessage*>(msg.get());
				    EXPECT_EQ(info->getNumberCore(), 1000);
			    });
			    session_on_server->read();
		    }).detach();

	    });

	std::thread client_1([&]() { // Клиент который что-то прочитал
		IOContextHandler client_context;
		Session session_client(client_context.getIOContext(), IOMode::Sync);
		session_client.connect(endpoint);
		session_client.setOnAllRead([](error_code ec, std::unique_ptr<Message>&& msg) {
			EXPECT_FALSE(ec);
			auto info = dynamic_cast<InformationMessage*>(msg.get());
			EXPECT_EQ(info->getNumberCore(), 1);
		});
		session_client.read();
	});

	std::thread client_3([&]() { // Клиент который что-то записал
		IOContextHandler client_context;
		Session session_client(client_context.getIOContext(), IOMode::Sync);
		session_client.connect(endpoint);
		session_client.write(std::make_unique<InformationMessage>("information", Transaction::Response, 1000));
	});
	client_1.join();
	client_3.join();
	server.join();
}
//
// TEST_F(SessionManagerTests, AsyncSessionManagerWith3Session)
// {
// 	sessionManager = std::make_shared<SessionManager>(IOMode::Async, endpoint);
//
// 	std::vector<std::shared_ptr<Session>> sessions;
// 	for(int i = 0; i < 3; i++) {
// 		auto session = std::make_shared<Session>(IOMode::Async);
// 		session->connect(endpoint);
// 		sessions.push_back(session);
// 	}
//
// 	std::thread([&]() {
// 		for(unsigned int sessionID = 0; sessionID < 3; sessionID++) {
// 			sessionManager->addSession(sessionID);
// 		}
// 	}).join();
//
// 	auto session_0 = sessionManager->getSession(0);
// 	session_0->setOnAllWrite([](error_code ec) {
// 		EXPECT_FALSE(ec);
// 	});
// 	session_0->write(std::make_unique<InformationMessage>("information", Transaction::Response, 1));
//
// 	auto session_1 = sessionManager->getSession(1);
// 	session_1->setOnAllRead([](error_code ec, std::unique_ptr<Message>&& msg) {
// 		EXPECT_FALSE(ec);
// 		auto info = dynamic_cast<InformationMessage*>(msg.get());
// 		EXPECT_EQ(info->getNumberCore(), 100);
// 	});
// 	session_1->read();
//
// 	sessions[0]->setOnAllRead([](error_code ec, std::unique_ptr<Message>&& msg) {
// 		EXPECT_FALSE(ec);
// 		auto info = dynamic_cast<InformationMessage*>(msg.get());
// 		EXPECT_EQ(info->getNumberCore(), 1);
// 	});
// 	sessions[0]->read();
//
// 	sessions[1]->setOnAllWrite([](error_code ec) {
// 		EXPECT_FALSE(ec);
// 	});
// 	sessions[1]->write(std::make_unique<InformationMessage>("information", Transaction::Response, 100));
//
// 	EXPECT_EQ(sessionManager->getSessionCount(), 3);
// 	EXPECT_EQ(sessions.size(), 3);
//