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
#include "TransportHandler.hpp"

#include "SyncClientConnectionHandler.hpp"

using namespace std::chrono_literals;

class SegmentServerTest : public ::testing::Test {
protected:

    void TearDown() override {
    	server->stop();
    	client->stop();
    }
    void startServer(bool multiConnect = true) {

        server = std::make_unique<SegmentServer>(address, port, multiConnect);
    	Logger::getInstance().setLevel(LogLevel::NoLog);
        server->start();

    	client = std::make_shared<SyncClientConnectionHandler>(address,port);
    	client->start();
    	client->connect();
    }


    boost::asio::io_context io_context;
	std::shared_ptr<SyncClientConnectionHandler> client = nullptr;
    std::unique_ptr<SegmentServer> server = nullptr;
	int port = 8000;
	std::string address = "127.0.0.1";
};


TEST_F(SegmentServerTest, StartStop_NoCrash) {
    startServer();
    EXPECT_TRUE(server->isRunning());
    server->stop();
    EXPECT_FALSE(server->isRunning());
}

TEST_F(SegmentServerTest, AcceptConnection_AndSendSignalRequest_ReturnsSignalResponse) {
    startServer();

    boost::json::object reqObj;
    reqObj["type"] = "signal";
    reqObj["transaction"] = 1;
    reqObj["central_Freq"] = 6100;
    reqObj["signal"] = boost::json::array{boost::json::array{-88.65925598144531, -65.49491882324219}};
    std::string jsonStr = boost::json::serialize(reqObj);
    std::vector<uint8_t> payload(jsonStr.begin(), jsonStr.end());

	client->setTaskSocket([&](ConnectedSocket& cs) {
		TransportHandler transport_handler(cs.ptr);
		MessageHandler message_handler;
		auto request = message_handler.serialize(std::make_unique<Message>("signal", Transaction::Request));
		EXPECT_TRUE(transport_handler.write(request));
		TransportMessage response = transport_handler.read();
		EXPECT_EQ(response.type, "signal");
		EXPECT_EQ(response.transaction, Transaction::Response);
		EXPECT_EQ(response.payload, payload);
	});

}

TEST_F(SegmentServerTest, InformationRequest_ReturnsInformationResponse) {
    startServer();

	boost::json::object reqObj;
	reqObj["type"] = "information";
	reqObj["transaction"] = 1;
	reqObj["numberCore"] = 4;
	std::string jsonStr = boost::json::serialize(reqObj);
	std::vector<uint8_t> payload(jsonStr.begin(), jsonStr.end());


	client->setTaskSocket([&](ConnectedSocket& cs) {
		TransportHandler transport_handler(cs.ptr);
		MessageHandler message_handler;
		auto request = message_handler.serialize(std::make_unique<Message>("information", Transaction::Request));
		EXPECT_TRUE(transport_handler.write(request));
		TransportMessage response = transport_handler.read();
		EXPECT_EQ(response.type, "information");
		EXPECT_EQ(response.transaction, Transaction::Response);
		EXPECT_EQ(response.payload, payload);
	});
}

TEST_F(SegmentServerTest, MultiConnectDisabled_OnlyOneConnectionAllowed) {
    startServer(false);
	// В startServer() уже сразу есть подключение
	auto client_two = std::make_shared<SyncClientConnectionHandler>(address, port);
	client_two->start();
	client_two->connect();

	client_two->setTaskSocket([](ConnectedSocket& cs) {
		TransportHandler transport_handler(cs.ptr);
		MessageHandler message_handler;
		auto request = message_handler.serialize(std::make_unique<Message>("information", Transaction::Request));
		EXPECT_FALSE(transport_handler.write(request));
	});

	client_two->stop();
}

// + TODO: убедиться, что не создаем больше потоков, чем есть сокетов в тесте
/// (отследить по логам либо (рекомендуется) по счетчиу вызова callback
TEST_F(SegmentServerTest, ChechCounterThread) {
    bool isMultiConnect = false;
    short unsigned NClient = 100;
    startServer(isMultiConnect);
    std::vector<std::shared_ptr<SyncClientConnectionHandler> > clients;
    for (int i = 0; i < NClient; i++) {
        auto client = std::make_shared<SyncClientConnectionHandler>(address, port);
        client->start();
        client->connect();
        clients.push_back(client);
    }

    if (isMultiConnect) {
        while (server->getConnectedClients().size() != NClient)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        EXPECT_EQ(server->getAliveThreads(), 2+NClient);
    } else {
        while (server->getConnectedClients().size() != 1)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));


        int waitCount = 0;
        while (server->getAliveThreads() != 3 && waitCount < 50) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            ++waitCount;
        }
        EXPECT_EQ(server->getAliveThreads(), 3);
    }
    for (auto client: clients)
        client->stop();
}
/// + TODO: тест записываем много сообщенй в сокет (следиим, чтобы они не успеи все обработаться), потом выполняем дисконнект
/*
* Что проверяет тест

    Сервер принимает одно соединение и создаёт отдельный поток обработки (aliveThreads увеличивается до 3).

    Клиент отправляет множество сообщений без пауз между отправками.

    Сразу после отправки сервер инициирует разрыв соединения через disconnect().

    Поток обработки сокета должен корректно завершиться: выход из цикла while (поскольку socket.ptr->is_open() станет false), уменьшение aliveThreads.

    Проверяем, что aliveThreads возвращается к значению 2 (только базовые потоки), а сервер остаётся работоспособен.
 */
TEST_F(SegmentServerTest, ManyMessagesThenDisconnect) {
    startServer(true);  // multiConnect = true

	boost::json::object reqObj;
	reqObj["type"] = "information";
	reqObj["transaction"] = static_cast<int>(Transaction::Request);
	reqObj["numberCore"] = 4;
	std::string jsonStr = boost::json::serialize(reqObj);
	std::vector<uint8_t> payload(jsonStr.begin(), jsonStr.end());
	TransportMessage request("information", Transaction::Request, payload);

    const int numMessages = 10;

    const int numMessagesBeforeDisconnect = numMessages/2;

    for (int i = 1; i <= numMessages; ++i) {
        if (i <= numMessagesBeforeDisconnect)
        	client->setTaskSocket([&request](ConnectedSocket& cs) {
        		TransportHandler transport_handler(cs.ptr);
        		ASSERT_TRUE(transport_handler.write(request));
        	});
        else
        	client->setTaskSocket([&request](ConnectedSocket& cs) {
				TransportHandler transport_handler(cs.ptr);
        		ASSERT_FALSE(transport_handler.write(request));
			});
        if (i == numMessagesBeforeDisconnect) {
        	server->disconnect(0);
        }
    }
	client->stop();
    while (server->getAliveThreads() != 2 && server->getConnectedClients().size() > 0) {
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