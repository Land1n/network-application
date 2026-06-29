//
// Created by ivan on 22.04.2026.
//
#include <gtest/gtest.h>

#include <boost/asio.hpp>
#include <thread>
#include <chrono>
#include <random>
#include <memory>

#include "segment_server/segment_server.h"
#include "session/session.h"

#include "io_context_handler/io_context_handler.h"
#include "message/signal_message.h"

#include "request_response_handler/request_response_handler.h"

class SegmentServerTest : public ::testing::Test {
protected:
	void TearDown() override {};
	void SetUp() override
	{
		server.setMagicNumber(700000);
		client.setMagicNumber(700000);
	};

	IOContextHandler io_handler;

	SegmentServer server{"127.0.0.1", 10000, true};
	Session client{io_handler.getIOContext(), IOMode::Sync};
};

TEST_F(SegmentServerTest, StartStop_NoCrash)
{
	Logger::getInstance().setLevel(LogLevel::Debug);

	server.start();
	EXPECT_TRUE(server.getIsWork());
	server.stop();
	EXPECT_FALSE(server.getIsWork());
}

TEST_F(SegmentServerTest, ConnectAndSendMessage)
{
	Logger::getInstance().setLevel(LogLevel::Debug);

	auto message = RequestResponseHandler::createRequest("signal");

	server.start();
	server.setReadHandler([&message](size_t, const void* data, size_t sz) {
		std::string str(static_cast<const char*>(data), sz - 1);
		auto js         = json::parse(str);
		auto signal_msg = dynamic_cast<const SignalMessage*>(message.get());
		SignalMessage read_msg("signal", Transaction::Request, js);
		EXPECT_EQ(read_msg.getSignal(), signal_msg->getSignal());
	});
	client.connect("127.0.0.1", 10000);
	client.write(message);
}
