//
// Created by ivan on 22.04.2026.
//
#include <gtest/gtest.h>

#include <boost/asio.hpp>
#include <thread>
#include <chrono>
#include <random>
#include <memory>

#include "SegmentServer.hpp"
#include "CreatorTransportHandler.hpp"

#include "SyncClientConnectionHandler.hpp"

using namespace std::chrono_literals;
// TODO: не понятно почему ломается теcт, при использовании valgrind пишет:
/*
* ==30759==  If you believe this happened as a result of a stack
==30759==  overflow in your program's main thread (unlikely but
==30759==  possible), you can try to increase the size of the
==30759==  main thread stack using the --main-stacksize= flag.
==30759==  The main thread stack size used in this run was 8388608.
 */
class SegmentServerTest : public ::testing::Test {
protected:
	void TearDown() override
	{
		if(server && server != nullptr)
			server->stop();
		if(client && client != nullptr)
			client->stop();
	}
	void SetUp() override
	{
		Logger::getInstance().setLevel(LogLevel::NoLog);
		client = std::make_shared<SyncClientConnectionHandler>(address, port);
	};
	void startServer(bool multiConnect = true)
	{
		server = std::make_unique<SegmentServer>(address, port, multiConnect);
		if(server && server != nullptr)
			server->start();
		else
			std::cerr << "SegmentServerTest: Failed to start server" << std::endl;
		if(client && client != nullptr) {
			client->start();
			client->connect();
		}
		else
			std::cerr << "SyncClientConnectionHandler: Failed to start server" << std::endl;
	}

	boost::asio::io_context io_context;
	std::shared_ptr<SyncClientConnectionHandler> client = nullptr;
	std::unique_ptr<SegmentServer> server               = nullptr;
	int port                                            = 8000;
	std::string address                                 = "127.0.0.1";
};

std::unique_ptr<BaseTransportHandler> createTransportHandler(const std::shared_ptr<tcp::socket>& socket)
{
	ParamsCreatorTransportHandler params;
	params.socket = socket;
	params.type   = TypeTransportHandler::Async;
	CreatorTransportHandler creator;
	return creator.create(params);
}

TEST_F(SegmentServerTest, StartStop_NoCrash)
{
	startServer();
	EXPECT_TRUE(server->isRunning());
	server->stop();
	EXPECT_FALSE(server->isRunning());
}

TEST_F(SegmentServerTest, AcceptConnection_AndSendSignalRequest_ReturnsSignalResponse)
{
	startServer();

	boost::json::object reqObj;
	reqObj["type"]         = "signal";
	reqObj["transaction"]  = 1;
	reqObj["central_Freq"] = 6100;
	reqObj["signal"]       = boost::json::array{boost::json::array{-88.65925598144531, -65.49491882324219}};
	std::string jsonStr    = boost::json::serialize(reqObj);
	std::vector<uint8_t> payload(jsonStr.begin(), jsonStr.end());

	client->setTaskSocket([&](ConnectedSocket& cs) {
		MessageHandler message_handler;
		auto request           = message_handler.serialize(std::make_unique<Message>("signal", Transaction::Request));
		auto transport_handler = createTransportHandler(cs.ptr);
		boost::system::error_code ec;
		transport_handler->write(request, ec);
		EXPECT_FALSE(ec);
		TransportMessage response;
		transport_handler->read(request, ec);
		EXPECT_EQ(response.type, "signal");
		EXPECT_EQ(response.transaction, Transaction::Response);
		EXPECT_EQ(response.payload, payload);
	});
}

TEST_F(SegmentServerTest, InformationRequest_ReturnsInformationResponse)
{
	startServer();

	boost::json::object reqObj;
	reqObj["type"]        = "information";
	reqObj["transaction"] = 1;
	reqObj["numberCore"]  = 4;
	std::string jsonStr   = boost::json::serialize(reqObj);
	std::vector<uint8_t> payload(jsonStr.begin(), jsonStr.end());

	client->setTaskSocket([&](ConnectedSocket& cs) {
		MessageHandler message_handler;
		auto request = message_handler.serialize(std::make_unique<Message>("information", Transaction::Request));

		auto transport_handler = createTransportHandler(cs.ptr);
		boost::system::error_code ec;
		transport_handler->write(request, ec);
		EXPECT_FALSE(ec);
		TransportMessage response;
		transport_handler->read(request, ec);
		EXPECT_EQ(response.type, "information");
		EXPECT_EQ(response.transaction, Transaction::Response);
		EXPECT_EQ(response.payload, payload);
	});
}

TEST_F(SegmentServerTest, MultiConnectDisabled_OnlyOneConnectionAllowed)
{
	startServer(false);
	// В startServer() уже сразу есть подключение
	auto client_two = std::make_shared<SyncClientConnectionHandler>(address, port);
	client_two->start();
	client_two->connect();

	client_two->setTaskSocket([](ConnectedSocket& cs) {
		MessageHandler message_handler;
		auto request = message_handler.serialize(std::make_unique<Message>("information", Transaction::Request));

		auto transport_handler = createTransportHandler(cs.ptr);
		boost::system::error_code ec;
		transport_handler->write(request, ec);
		EXPECT_FALSE(ec);
	});

	client_two->stop();
}

// + TODO: убедиться, что не создаем больше потоков, чем есть сокетов в тесте
/// (отследить по логам либо (рекомендуется) по счетчиу вызова callback
TEST_F(SegmentServerTest, ChechCounterThread)
{
	bool isMultiConnect    = false;
	short unsigned NClient = 100;
	startServer(isMultiConnect);
	std::vector<std::shared_ptr<SyncClientConnectionHandler>> clients;
	for(int i = 0; i < NClient; i++) {
		auto client = std::make_shared<SyncClientConnectionHandler>(address, port);
		client->start();
		client->connect();
		clients.push_back(client);
	}

	if(isMultiConnect) {
		while(server->getConnectedClients().size() != NClient)
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		EXPECT_EQ(server->getAliveThreads(), 2 + NClient);
	}
	else {
		while(server->getConnectedClients().size() != 1)
			std::this_thread::sleep_for(std::chrono::milliseconds(1));

		int waitCount = 0;
		while(server->getAliveThreads() != 3 && waitCount < 50) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			++waitCount;
		}
		EXPECT_EQ(server->getAliveThreads(), 3);
	}
	for(auto client: clients)
		client->stop();
}
/// + TODO: тест записываем много сообщенй в сокет (следиим, чтобы они не успеи все обработаться), потом выполняем
/// дисконнект
/*
* Что проверяет тест

    Сервер принимает одно соединение и создаёт отдельный поток обработки (aliveThreads увеличивается до 3).

    Клиент отправляет множество сообщений без пауз между отправками.

    Сразу после отправки сервер инициирует разрыв соединения через disconnect().

    Поток обработки сокета должен корректно завершиться: выход из цикла while (поскольку socket.ptr->is_open() станет
false), уменьшение aliveThreads.

    Проверяем, что aliveThreads возвращается к значению 2 (только базовые потоки), а сервер остаётся работоспособен.
 */
TEST_F(SegmentServerTest, ManyMessagesThenDisconnect)
{
	startServer(true); // multiConnect = true
	std::cerr << 0 << " ";
	boost::json::object reqObj;
	reqObj["type"]        = "information";
	reqObj["transaction"] = static_cast<int>(Transaction::Request);
	reqObj["numberCore"]  = 4;
	std::string jsonStr   = boost::json::serialize(reqObj);
	std::vector<uint8_t> payload(jsonStr.begin(), jsonStr.end());
	TransportMessage request("information", Transaction::Request, payload);

	const int numMessages = 10;

	const int numMessagesBeforeDisconnect = numMessages / 2;

	for(int i = 1; i <= numMessages; ++i) {
		if(i <= numMessagesBeforeDisconnect)
			client->setTaskSocket([&request](ConnectedSocket& cs) {
				auto transport_handler = createTransportHandler(cs.ptr);
				boost::system::error_code ec;
				if(transport_handler && transport_handler != nullptr) {
					transport_handler->write(request, ec);
					EXPECT_FALSE(ec);
				}
				else
					FAIL();
			});
		else
			client->setTaskSocket([&request](ConnectedSocket& cs) {
				auto transport_handler = createTransportHandler(cs.ptr);
				boost::system::error_code ec;
				transport_handler->write(request, ec);
				if(transport_handler && transport_handler != nullptr) {
					transport_handler->write(request, ec);
					EXPECT_TRUE(ec);
				}
				else
					FAIL();
			});
		if(i == numMessagesBeforeDisconnect) {
			server->disconnect(0);
		}
	}
	client->stop();
	while(server->getAliveThreads() != 2 && server->getConnectedClients().size() > 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	EXPECT_EQ(server->getAliveThreads(), 2);

	EXPECT_TRUE(server->isRunning());

	auto new_client = std::make_shared<SyncClientConnectionHandler>(address, port);
	new_client->start();
	new_client->connect();

	server->setReadHandler([](Network::ConnectionId, const void* data, size_t sz) {
		json::value json_value = json::parse(static_cast<const char*>(data));
		EXPECT_EQ(json_value.at("type"), "information");
		EXPECT_EQ(json_value.at("transaction"), 1);
	});
	new_client->setTaskSocket([&request](ConnectedSocket& cs) {
		TransportHandler transport_handler(cs.ptr);
		ASSERT_TRUE(transport_handler.write(request));
	});

	new_client->stop();
}