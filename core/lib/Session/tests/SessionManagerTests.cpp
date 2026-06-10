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

	session->setOnConnect([&sessionManager](error_code) {
		EXPECT_EQ(sessionManager.getSessionCount(), 1);
	});
	session->connect(endpoint);

	session->disconnect();
	sessionManager.getSession(0)->read();
	EXPECT_EQ(sessionManager.getSessionCount(), 0);
}

TEST_F(SessionManagerTests, AsyncErrorAndRemove)
{
	std::promise<void> add_done;
	SessionManager sessionManager(server_context.getIOContext(), IOMode::Async, endpoint);
	std::thread([&sessionManager, &add_done]() {
		sessionManager.addSession(0);
		add_done.set_value();
	}).detach();

	Session session_client(client_context.getIOContext(), IOMode::Async);
	sessionManager.setOnAccept([&sessionManager](error_code) {
		EXPECT_EQ(sessionManager.getSessionCount(), 1);
	});
	session_client.connect(endpoint);

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

// // Тест на имитацию 3 подключений
// TEST_F(SessionManagerTests, SyncSessionManagerWith3Session)
// {
// 	SessionManager sessionManager(server_context.getIOContext(), IOMode::Sync, endpoint);
//
// 	std::thread server(
// 	    [&sessionManager]() { // Почему поток создается в потоке? Потому что для каждого пользователя свой поток
// 		    std::thread([&]() {
// 			    sessionManager.addSession(0);
// 			    auto session_on_server = sessionManager.getSession(0);
// 			    session_on_server->write(std::make_unique<InformationMessage>("information", Transaction::Response, 1));
// 		    }).detach();
//
// 		    std::thread([&]() {
// 			    sessionManager.addSession(2);
// 			    auto session_on_server = sessionManager.getSession(2);
// 			    session_on_server->setOnAllRead([](error_code ec, std::unique_ptr<Message>&& msg) {
// 				    EXPECT_FALSE(ec);
// 				    auto info = dynamic_cast<InformationMessage*>(msg.get());
// 				    EXPECT_EQ(info->getNumberCore(), 1);
// 			    });
// 			    session_on_server->read();
// 		    }).detach();
//
// 	    });
//
// 	std::thread client_1([&]() { // Клиент который что-то прочитал
// 		IOContextHandler client_context;
// 		Session session_client(client_context.getIOContext(), IOMode::Sync);
// 		session_client.connect(endpoint);
// 		session_client.setOnAllRead([](error_code ec, std::unique_ptr<Message>&& msg) {
// 			EXPECT_FALSE(ec);
// 			auto info = dynamic_cast<InformationMessage*>(msg.get());
// 			EXPECT_EQ(info->getNumberCore(), 1);
// 		});
// 		session_client.read();
// 	});
//
// 	std::thread client_3([&]() { // Клиент который что-то записал
// 		IOContextHandler client_context;
// 		Session session_client(client_context.getIOContext(), IOMode::Sync);
// 		session_client.connect(endpoint);
// 		session_client.write(std::make_unique<InformationMessage>("information", Transaction::Response, 1));
// 	});
// 	client_1.join();
// 	client_3.join();
// 	server.join();
// }
// TEST_F(SessionManagerTests, AsyncSessionManagerWith3Session)
// {
// 	SessionManager sessionManager(server_context.getIOContext(), IOMode::Async, endpoint);
//
// 	std::promise<void> read_done;
// 	std::promise<void> write_done;
//
// 	sessionManager.addSession(0);
// 	auto session_on_server = sessionManager.getSession(0);
// 	session_on_server->setOnAllWrite([&write_done](error_code ec) {
// 		write_done.set_value();
// 	});
// 	session_on_server->setOnAccept([&](error_code ec) {
// 		session_on_server->write(std::make_unique<InformationMessage>("information", Transaction::Response, 1));
// 	});
// 	IOContextHandler client_context;
// 	Session session_client(client_context.getIOContext(), IOMode::Async);
//
// 	session_client.setOnAllRead([&read_done](error_code ec, std::unique_ptr<Message>&& msg) {
// 		EXPECT_FALSE(ec);
// 		auto info = dynamic_cast<InformationMessage*>(msg.get());
// 		EXPECT_EQ(info->getNumberCore(), 1);
// 		read_done.set_value();
// 	});
// 	session_client.setOnConnect([&session_client](error_code ec) {
// 		EXPECT_FALSE(ec);
// 		session_client.read();
// 	});
// 	session_client.connect(endpoint);
//
// 	read_done.get_future().wait();
// 	write_done.get_future().wait();
// }