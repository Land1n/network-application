//
// Created by guestuser on 01.06.2026.
//
#include "Session/Session.hpp"

#include <gtest/gtest.h>

#include "AcceptHandler/AcceptHandler.hpp"
// TODO: + Enum
// TODO: Блоки кода
// TODO: +- sync vs async

// TODO: hasError ( при при обращении к сесси если true удаляем из SM)
// TODO: + Убрвть future
// TODO: тесты на последовательную запись и чтение

class SessionTests : public ::testing::Test {
public:
	boost::asio::io_context io;
	tcp::endpoint endpoint = tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), 8100);
	std::unique_ptr<tcp::socket> server_socket;
	std::shared_ptr<AcceptHandler> acceptHandler;
	void SetUp() override
	{
		acceptHandler = std::make_shared<AcceptHandler>(io, endpoint);
		server_socket = std::make_unique<tcp::socket>(io);
	}
	void TearDown() override
	{
		error_code ec;
		acceptHandler->close();
	}
	static void expect_success(error_code ec) { EXPECT_FALSE(ec); }
	static void expect_failure(error_code ec) { EXPECT_TRUE(ec); }
};

TEST_F(SessionTests, SyncCreateSessionAndConnection)
{
	acceptHandler->setOnAccept(expect_success);
	acceptHandler->accept(*server_socket,IOMode::Async);
	auto session = std::make_shared<Session>(IOMode::Sync);
	session->setOnConnect(expect_success);
	session->connect(endpoint);
	io.run();
}
TEST_F(SessionTests, AsyncCreateSessionAndConnection)
{
	acceptHandler->setOnAccept(expect_success);
	acceptHandler->accept(*server_socket,IOMode::Async);
	auto session = std::make_shared<Session>(IOMode::Async);
	session->setOnConnect(expect_success);
	session->connect(endpoint);
	io.run();
}
TEST_F(SessionTests, SyncSessionWrite)
{
	auto req = std::make_unique<Message>("signal", Transaction::Request);
	acceptHandler->setOnAccept([&](error_code ec) {
		EXPECT_FALSE(ec);
		char c[1];
		error_code code;
		boost::asio::read(*server_socket, boost::asio::buffer(c, 1), code);
		EXPECT_FALSE(code);
	});
	acceptHandler->accept(*server_socket,IOMode::Async);
	auto session = std::make_shared<Session>(IOMode::Sync);
	session->setOnConnect(expect_success);
	session->connect(endpoint);
	session->write(req);
	io.run();
}
TEST_F(SessionTests, AsyncSessionWrite)
{
	auto req = std::make_unique<Message>("signal", Transaction::Request);
	acceptHandler->setOnAccept([&](error_code ec) {
		EXPECT_FALSE(ec);
		char c[1];
		error_code code;
		boost::asio::read(*server_socket, boost::asio::buffer(c, 1), code);
		EXPECT_FALSE(code);
	});
	acceptHandler->accept(*server_socket,IOMode::Async);
	auto session = std::make_shared<Session>(IOMode::Async);
	session->setOnConnect([&](error_code ec) {
		EXPECT_FALSE(ec);
		session->write(req);
	});
	session->connect(endpoint);
	io.run();
}