//
// Created by ivan on 07.03.2026.
//
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
	std::unique_ptr<tcp::acceptor> acceptor;
	tcp::endpoint endpoint = tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), 8090);
	std::unique_ptr<tcp::socket> client_socket;
	std::unique_ptr<tcp::socket> server_socket;
	void SetUp() override
	{
		acceptor = std::make_unique<tcp::acceptor>(server_context);
		acceptor->open(endpoint.protocol());
		acceptor->set_option(tcp::acceptor::reuse_address(true));
		acceptor->bind(endpoint);
		acceptor->listen();

		server_socket = std::make_unique<tcp::socket>(server_context);
		client_socket = std::make_unique<tcp::socket>(client_context);
		std::thread([&]() {
			acceptor->accept(*server_socket);
		}).detach();
	}
	void TearDown() override
	{
		if (acceptor && acceptor->is_open()) {
			boost::system::error_code ec;
			acceptor->cancel(ec);
			acceptor->close(ec);
		}
		if (server_socket && server_socket->is_open()) {
			boost::system::error_code ec;
			server_socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
			server_socket->close(ec);
		}

		client_context.poll();
		server_context.poll();

		acceptor.reset();
		server_socket.reset();
		client_socket.reset();
	}
	std::shared_ptr<ConnectionHandler> create_connection_handler(IOMode mode,const CallBack& callback)
	{
		auto connection_handler = std::make_shared<ConnectionHandler>(*client_socket);
		connection_handler->setOnConnect(callback);
		error_code ec;
		connection_handler->connect(endpoint,mode);
		client_context.run();
		return connection_handler;
	}
	std::shared_ptr<ConnectionHandler> create_connection_handler(IOMode type)
	{
		return create_connection_handler(type,[](error_code) {});
	}

	TransportMessage create_test_message()
	{
		std::string json_str = R"({"type":"test","transaction":2})";
		std::vector<uint8_t> payload(json_str.begin(), json_str.end());
		return TransportMessage("test",Transaction::Tests,payload);
	};

};

TEST_F(TransportHandlerTests, SyncWriteSuccsesful)
{
	auto con = create_connection_handler(IOMode::Sync);
	error_code error;

	TransportHandler transport_handler(*client_socket);
	auto f_ec = transport_handler.write(create_test_message(),IOMode::Sync);

	EXPECT_FALSE(f_ec.get());

}

TEST_F(TransportHandlerTests, AsyncWriteSuccsesful)
{
	TransportHandler transport_handler(*client_socket);
	std::future<error_code> f_ec;
	create_connection_handler(IOMode::Async,[&](error_code error) {
		f_ec = transport_handler.write(create_test_message(),IOMode::Async);
		EXPECT_FALSE(error);
	});
	EXPECT_FALSE(f_ec.get());
}
TEST_F(TransportHandlerTests, SyncWriteAndReadSuccsesful)
{

	auto con = create_connection_handler(IOMode::Sync);
	TransportMessage for_write_message = create_test_message();
	error_code error;

	TransportHandler transport_handler(*client_socket);
	auto f_ec_w = transport_handler.write(for_write_message,IOMode::Sync);

	TransportHandler transport_handler_server(*server_socket);
	TransportMessage for_read_message;

	auto f_ec_r = transport_handler_server.read(for_read_message,IOMode::Sync);
	EXPECT_FALSE(f_ec_w.get());
	EXPECT_FALSE(f_ec_r.get());
	EXPECT_EQ(for_read_message.type, for_write_message.type);
	EXPECT_EQ(for_read_message.transaction, for_write_message.transaction);
	EXPECT_EQ(for_read_message.payload, for_write_message.payload);
}

TEST_F(TransportHandlerTests, AsyncWriteAndReadSuccsesful)
{

	TransportHandler transport_handler(*client_socket);
	auto for_write_message = create_test_message();
	std::future<error_code> f_ec_w;
	create_connection_handler(IOMode::Async,[&](error_code error) {
		f_ec_w = transport_handler.write(for_write_message,IOMode::Async);
		EXPECT_FALSE(error);
	});
	client_context.run();
	error_code error;
	TransportHandler transport_handler_server(*server_socket);
	TransportMessage for_read_message;

	auto f_ec_r = transport_handler_server.read(for_read_message,IOMode::Async);
	server_context.run();
	EXPECT_FALSE(f_ec_w.get());
	EXPECT_FALSE(f_ec_r.get());
	EXPECT_EQ(for_read_message.type, for_write_message.type);
	EXPECT_EQ(for_read_message.transaction, for_write_message.transaction);
	EXPECT_EQ(for_read_message.payload, for_write_message.payload);
}

// TEST_F(TransportHandlerTests, SendAndReadOnSocketTest)
// {
// 	std::vector<uint8_t> test_data = {1, 2, 3, 4, 8};
//
//
// 	server->setTaskSocket([&](ConnectedSocket& cs) {
// 		boost::system::error_code error;
// 		auto handler = CreateTransportHandler(cs.ptr);
// 		TransportMessage msg;
// 		handler->read(msg,error);
// 		EXPECT_EQ(msg.payload, test_data);
// 	});
//
// 	client->setTaskSocket([&](ConnectedSocket& cs) {
// 		boost::system::error_code error;
// 		auto handler = CreateTransportHandler(cs.ptr);
// 		TransportMessage msg;
// 		msg.payload = test_data;
// 		handler->write(msg,error);
// 	});
// }
// // Тест на несколько последовательных сообщений
// TEST_F(TransportHandlerTests, MultipleMessages)
// {
// 	std::atomic<int> received_count{0};
// 	const int expected_count = 5;
//
// 	server->setTaskSocket([&](ConnectedSocket& cs) {
// 		auto transport  = CreateTransportHandler(cs.ptr);
// 		for(int i = 0; i < expected_count; ++i) {
// 			TransportMessage msg;
// 			boost::system::error_code error_code;
//
// 			transport->read(msg,error_code);
// 			if(!msg.type.empty()) {
// 				received_count++;
// 			}
// 		}
// 	});
//
// 	auto client_sock = client->connect();
// 	ASSERT_NE(client_sock.ptr, nullptr);
// 	auto client_transport = CreateTransportHandler(client_sock.ptr);
//
// 	for(int i = 0; i < expected_count; ++i) {
// 		boost::system::error_code error_code;
// 		boost::json::object obj;
// 		obj["type"]          = "msg" + std::to_string(i);
// 		obj["transaction"]   = 0;
// 		std::string json_str = boost::json::serialize(obj);
// 		TransportMessage msg;
// 		msg.type        = "msg" + std::to_string(i);
// 		msg.transaction = Transaction::Request;
// 		msg.payload.assign(json_str.begin(), json_str.end());
// 		client_transport->write(msg,error_code);
// 	}
// 	while(expected_count != received_count.load())
// 		std::this_thread::sleep_for(std::chrono::milliseconds(10));
// 	EXPECT_EQ(received_count.load(), expected_count);
// }
//
// // Тест на обрыв соединения во время чтения
// TEST_F(TransportHandlerTests, ConnectionLostDuringRead)
// {
// 	bool isStartRead = false;
//
// 	server->setTaskSocket([&](ConnectedSocket& cs) {
// 		auto transport = CreateTransportHandler(cs.ptr);
// 		boost::system::error_code error;
// 		isStartRead = true;
// 		TransportMessage msg;
// 		transport->read(msg,error);
// 		EXPECT_EQ(msg.type, "error");
// 		EXPECT_EQ(msg.transaction, Transaction::Error);
// 	});
//
// 	auto client_sock = client->connect();
// 	ASSERT_NE(client_sock.ptr, nullptr);
//
// 	uint32_t magic = 0xA0ABA0A;
// 	boost::asio::write(*client_sock.ptr, boost::asio::buffer(&magic, 4));
// 	while(!isStartRead) std::this_thread::sleep_for(std::chrono::microseconds(100));
// 	client_sock.ptr->close();
// }

// Стресс-тест: множество клиентов обмениваются сообщениями через TransportHandler

// TEST_F(TransportHandlerTests, ManyClientsMessaging)
// {
// 	const int num_clients         = 10;
// 	const int messages_per_client = 10;
//
// 	std::atomic<int> server_handlers_active{0};
// 	std::mutex server_handlers_mutex;
// 	std::condition_variable server_handlers_cv;
// 	server->setTaskSocket([&](ConnectedSocket& cs) {
// 		server_handlers_active++;
// 		auto transport = CreateTransportHandler(cs.ptr);
// 		try {
// 			while(true) {
// 				boost::system::error_code error;
// 				TransportMessage req;
// 				transport->read(req,error);
//
// 				if(req.transaction == Transaction::Error)
// 					break;
//
// 				if(req.type == "ping") {
// 					std::string json_str(req.payload.begin(), req.payload.end());
// 					boost::json::value jv = boost::json::parse(json_str);
// 					std::string data      = boost::json::value_to<std::string>(jv.at("data"));
//
// 					// Формируем ответ
// 					std::string response_data = "pong " + data;
// 					boost::json::object resp_obj;
// 					resp_obj["type"]        = "pong";
// 					resp_obj["transaction"] = static_cast<int>(Transaction::Response);
// 					resp_obj["data"]        = response_data;
// 					std::string resp_json   = boost::json::serialize(resp_obj);
// 					std::vector<uint8_t> resp_payload(resp_json.begin(), resp_json.end());
// 					TransportMessage resp("pong", Transaction::Response, resp_payload);
// 					transport->write(resp,error);
// 				}
// 			}
// 		}
// 		catch(const std::exception&) {
// 		}
// 		server_handlers_active--;
// 		server_handlers_cv.notify_one();
// 	});
//
// 	std::atomic<int> clients_finished{0};
// 	std::mutex finish_mutex;
// 	std::condition_variable finish_cv;
// 	std::vector<std::shared_ptr<SyncClientConnectionHandler>> clients;
// 	clients.reserve(num_clients);
//
// 	auto start_time = std::chrono::steady_clock::now();
//
// 	for(int i = 0; i < num_clients; ++i) {
// 		auto client = std::make_shared<SyncClientConnectionHandler>(address, port);
//
// 		client->start();
//
// 		client->setTaskSocket([&](ConnectedSocket& cs) {
// 			auto transport = CreateTransportHandler(cs.ptr);
// 			try {
// 				for(int msg_id = 1; msg_id <= messages_per_client; ++msg_id) {
// 					// Формируем JSON-запрос
// 					boost::system::error_code error;
//
// 					boost::json::object req_obj;
// 					req_obj["type"]        = "ping";
// 					req_obj["transaction"] = static_cast<int>(Transaction::Request);
// 					req_obj["data"]        = "ping " + std::to_string(msg_id);
// 					std::string req_json   = boost::json::serialize(req_obj);
// 					std::vector<uint8_t> req_payload(req_json.begin(), req_json.end());
// 					TransportMessage req("ping", Transaction::Request, req_payload);
// 					transport->write(req,error);
// 					if(error) break;
//
// 					// Читаем ответ
// 					TransportMessage resp;
// 					transport->read(resp,error);
// 					if(resp.type != "pong")
// 						break;
// 					std::string resp_json(resp.payload.begin(), resp.payload.end());
// 					boost::json::value jv     = boost::json::parse(resp_json);
// 					std::string response_data = boost::json::value_to<std::string>(jv.at("data"));
// 					std::string expected      = "pong ping " + std::to_string(msg_id);
// 					EXPECT_EQ(response_data, expected);
// 				}
// 				// Закрываем соединение, чтобы серверный обработчик вышел из цикла
// 				cs.ptr->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
// 				cs.ptr->close();
// 			}
// 			catch(const std::exception&) {
// 				// игнорируем
// 			}
// 			clients_finished++;
// 			finish_cv.notify_one();
// 		});
//
// 		auto sock = client->connect();
// 		ASSERT_NE(sock.ptr, nullptr);
// 		clients.push_back(client);
// 	}
//
// 	{
// 		std::unique_lock<std::mutex> lock(server_handlers_mutex);
// 		bool all_handlers_done = server_handlers_cv.wait_for(lock, std::chrono::seconds(3), [&]() {
// 			return server_handlers_active.load() == 0;
// 		});
// 		EXPECT_TRUE(all_handlers_done) << "Server handlers did not finish";
// 	}
//
// 	auto duration = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_time);
// 	std::cout << "ManyClientsMessaging completed with " << num_clients << " clients, " << messages_per_client
// 	          << " messages each in " << duration.count() << " seconds\n";
//
// 	for(auto& c: clients) {
// 		c->stop();
// 	}
// }