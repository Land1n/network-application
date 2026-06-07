//
// Created by ivan on 07.03.2026.
//
#include "InformationMessage.hpp"
#include "MessageHandler.hpp"
#include "RawMessage.hpp"
#include "AcceptHandler/AcceptHandler.hpp"
#include <boost/asio.hpp>
#include <gtest/gtest.h>
#include <thread>
#include <boost/json.hpp>
#include "ConnectionHandler/ConnectionHandler.hpp"

#include "TransportHandler/TransportHandler.hpp"

namespace json = boost::json;

class TransportHandlerTests : public ::testing::Test {
public:
	boost::asio::io_context client_context;
	boost::asio::io_context server_context;
	tcp::endpoint endpoint = tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), 8090);
	std::shared_ptr<tcp::socket> client_socket;
	std::shared_ptr<tcp::socket> server_socket;

	std::shared_ptr<ConnectionHandler> connection_handler;
	std::shared_ptr<AcceptHandler> accept_handler;

	void SetUp() override
	{
		server_socket = std::make_shared<tcp::socket>(server_context);
		client_socket = std::make_shared<tcp::socket>(client_context);

		accept_handler     = std::make_shared<AcceptHandler>(server_context, endpoint);
		connection_handler = std::make_shared<ConnectionHandler>(client_socket);
	}
	void TearDown() override
	{
		boost::system::error_code ec;
		server_socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
		server_socket->close(ec);

		client_context.poll();
		server_context.poll();

		server_socket.reset();
		client_socket.reset();
	}
	TransportMessage create_test_message()
	{
		std::string json_str = R"({"type":"test","transaction":2})";
		std::vector<uint8_t> payload(json_str.begin(), json_str.end());
		return TransportMessage("test", Transaction::Tests, payload);
	};

	TransportMessage create_info_message(int id)
	{
		auto message = std::make_unique<InformationMessage>("information", Transaction::Response, id);
		MessageHandler handler;
		return handler.serialize(std::move(message));
	};
	int get_data_message(TransportMessage&& transportMessage)
	{
		MessageHandler handler;
		auto message      = handler.parse(std::move(transportMessage));
		auto info_message = dynamic_cast<InformationMessage*>(message.get());
		return info_message->getNumberCore();
	}

	static void expect_success_with_tm(error_code ec, TransportMessage tm)
	{
		EXPECT_FALSE(ec);
	}
	static void expect_success(error_code ec)
	{
		EXPECT_FALSE(ec);
	}

	static void expect_failure(error_code ec, TransportMessage tm)
	{
		EXPECT_TRUE(ec);
	}
};
//
TEST_F(TransportHandlerTests, SyncWriteSuccsesful)
{
	connection_handler->connect(endpoint, IOMode::Sync);
	accept_handler->accept(server_socket, IOMode::Sync);

	TransportHandler transport_handler(client_socket);
	transport_handler.setOnAllWrite(expect_success_with_tm);
	transport_handler.write(create_test_message(), IOMode::Sync);
}

TEST_F(TransportHandlerTests, AsyncWriteSuccsesful)
{
	TransportHandler transport_handler(client_socket);
	transport_handler.setOnAllWrite(expect_success_with_tm);
	connection_handler->setOnConnect([&](error_code error) {
		transport_handler.write(create_test_message(), IOMode::Async);
		EXPECT_FALSE(error);
	});
	accept_handler->setOnAccept(expect_success);

	connection_handler->connect(endpoint, IOMode::Async);
	accept_handler->accept(server_socket, IOMode::Async);
	server_context.run();
	client_context.run();
}
TEST_F(TransportHandlerTests, SyncWriteAndReadSuccsesful)
{
	connection_handler->setOnConnect(expect_success);
	accept_handler->setOnAccept(expect_success);
	connection_handler->connect(endpoint, IOMode::Sync);
	accept_handler->accept(server_socket, IOMode::Sync);

	TransportHandler transport_handler(client_socket);
	transport_handler.setOnAllWrite(expect_success_with_tm);
	transport_handler.write(create_test_message(), IOMode::Sync);

	TransportHandler transport_handler_server(server_socket);
	transport_handler_server.setOnAllRead(
	    [for_write_message = create_test_message()](error_code error, TransportMessage&& for_read_message) {
		    EXPECT_FALSE(error);
		    EXPECT_EQ(for_read_message.type, for_write_message.type);
		    EXPECT_EQ(for_read_message.transaction, for_write_message.transaction);
		    EXPECT_EQ(for_read_message.payload, for_write_message.payload);
	    });
	transport_handler_server.read(IOMode::Sync);
}

TEST_F(TransportHandlerTests, AsyncWriteAndReadSuccsesful)
{
	TransportHandler transport_handler_client(client_socket);
	TransportHandler transport_handler_server(server_socket);

	transport_handler_client.setOnAllWrite(expect_success_with_tm);

	connection_handler->setOnConnect([&](error_code error) {
		transport_handler_client.write(create_test_message(), IOMode::Async);
		EXPECT_FALSE(error);
	});
	transport_handler_server.setOnAllRead(
	    [for_write_message = create_test_message()](error_code error, TransportMessage for_read_message) {
		    EXPECT_FALSE(error);
		    EXPECT_EQ(for_read_message.type, for_write_message.type);
		    EXPECT_EQ(for_read_message.transaction, for_write_message.transaction);
		    EXPECT_EQ(for_read_message.payload, for_write_message.payload);
	    });
	accept_handler->setOnAccept([&transport_handler_server](error_code error) {
		EXPECT_FALSE(error);
		transport_handler_server.read(IOMode::Async);
	});
	connection_handler->setOnConnect([&](error_code error) {
		EXPECT_FALSE(error);
		transport_handler_client.write(create_test_message(), IOMode::Async);
	});
	accept_handler->accept(server_socket, IOMode::Async);
	connection_handler->connect(endpoint, IOMode::Async);
	std::thread([&]() {
		server_context.run();
	}).detach();
	/// TODO:
	auto work = boost::asio::make_work_guard(client_context);
	client_context.run_for(std::chrono::milliseconds(3));
}

TEST_F(TransportHandlerTests, SyncWriteAndError)
{
	std::atomic<bool> isError = false;

	TransportHandler transport_handler(client_socket);
	transport_handler.setOnError([&isError](error_code error) {
		isError.store(true);
	});
	transport_handler.write(create_test_message(), IOMode::Sync);
	EXPECT_TRUE(isError.load());
}

TEST_F(TransportHandlerTests, AsyncWriteAndError)
{
	std::atomic<bool> isError = false;

	TransportHandler transport_handler(client_socket);
	transport_handler.setOnError([&isError](error_code error) {
		isError = true;
	});
	transport_handler.write(create_test_message(), IOMode::Async);
	client_context.run();

	EXPECT_TRUE(isError.load());
}

TEST_F(TransportHandlerTests, SyncWriteNAndReadN)
{
	const short N = 1000;
	std::atomic<short> N_write{0};
	std::atomic<short> N_read{0};
	std::vector<int> ints;
	std::mutex ints_mutex;

	accept_handler->setOnAccept([](error_code ec) {
		EXPECT_FALSE(ec);
	});

	connection_handler->setOnConnect([](error_code ec) {
		EXPECT_FALSE(ec);
	});

	std::thread server_thread([&]() {
		accept_handler->accept(server_socket, IOMode::Sync);
		TransportHandler th_server(server_socket);
		th_server.setOnAllRead([&](error_code ec, TransportMessage&& msg) {
			EXPECT_FALSE(ec);
			int u = get_data_message(std::move(msg));
			{
				std::lock_guard<std::mutex> lock(ints_mutex);
				auto it = std::find(ints.begin(), ints.end(), u);
				EXPECT_NE(it, ints.end());
			}
			N_read++;
		});
		for(int i = 0; i < N; ++i) {
			th_server.read(IOMode::Sync);
		}
	});

	connection_handler->connect(endpoint, IOMode::Sync);
	TransportHandler th_client(client_socket);
	th_client.setOnAllWrite([&](error_code ec, TransportMessage) {
		EXPECT_FALSE(ec);
		{
			std::lock_guard<std::mutex> lock(ints_mutex);
			ints.push_back(N_write);
		}
		N_write++;
	});
	for(int i = 0; i < N; ++i) {
		th_client.write(create_info_message(i), IOMode::Sync);
	}

	server_thread.join();

	EXPECT_EQ(N_read, N);
	EXPECT_EQ(N_write, N);
}

TEST_F(TransportHandlerTests, AsyncWriteNAndReadN)
{
	const short N = 1000;
	std::atomic<short> N_write{0};
	std::atomic<short> N_read{0};
	std::vector<int> ints;
	std::mutex ints_mutex;

	TransportHandler th_client(client_socket);
	TransportHandler th_server(server_socket);

	th_client.setOnAllWrite([&](error_code error, TransportMessage) {
		EXPECT_FALSE(error);
		{
			std::lock_guard<std::mutex> lock(ints_mutex);
			ints.push_back(N_write);
		}
		N_write++;
		if(N_write < N) {
			th_client.write(create_info_message(N_write), IOMode::Async);
		}
	});

	th_server.setOnAllRead([&](error_code error, TransportMessage msg) {
		EXPECT_FALSE(error);
		int u = get_data_message(std::move(msg));
		{
			std::lock_guard<std::mutex> lock(ints_mutex);
			auto it = std::find(ints.begin(), ints.end(), u);
			EXPECT_NE(it, ints.end());
		}
		N_read++;
		if(N_read < N) {
			th_server.read(IOMode::Async);
		}
	});

	connection_handler->setOnConnect([&](error_code error) {
		EXPECT_FALSE(error);
		th_client.write(create_info_message(0), IOMode::Async);
	});
	accept_handler->setOnAccept([&](error_code error) {
		EXPECT_FALSE(error);
		th_server.read(IOMode::Async);
	});
	connection_handler->connect(endpoint, IOMode::Async);
	accept_handler->accept(server_socket, IOMode::Async);
	std::thread t2([&]() {
		client_context.run();
	});

	std::thread t1([&]() {
		server_context.run();
	});
	t1.join();
	t2.join();
	EXPECT_EQ(N_read, N_write);
}
