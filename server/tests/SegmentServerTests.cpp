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

using namespace std::chrono_literals;

class SegmentServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(20000, 30000);
        port = dist(gen);
        address = "127.0.0.1";
    }

    void TearDown() override {
        if (server) {
            server->stop();
            server.reset();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    void startServer(bool multiConnect = true) {
        server = std::make_unique<SegmentServer>(address, port, multiConnect, false);
        server->start();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::unique_ptr<TransportHandler> connectClient() {
        auto sock = std::make_shared<boost::asio::ip::tcp::socket>(io_context);
        boost::system::error_code ec;
        sock->connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(address), port), ec);
        if (ec) return nullptr;
        return std::make_unique<TransportHandler>(sock);
    }

    boost::asio::io_context io_context;
    std::unique_ptr<SegmentServer> server = nullptr;
    std::string address;
    int port;
};

TransportMessage sendAndReceive(TransportHandler &client, TransportMessage request) {
    EXPECT_TRUE(client.write(request));
    return client.read();
}

TEST_F(SegmentServerTest, StartStop_NoCrash) {
    startServer();
    EXPECT_TRUE(server->isRunning());
    server->stop();
    EXPECT_FALSE(server->isRunning());
}

TEST_F(SegmentServerTest, AcceptConnection_AndSendSignalRequest_ReturnsSignalResponse) {
    startServer();

    auto clientHandler = connectClient();
    ASSERT_NE(clientHandler, nullptr);

    // Создаём запрос signal
    boost::json::object reqObj;
    reqObj["type"] = "signal";
    reqObj["transaction"] = 1; // Request
    reqObj["central_Freq"] = 6100;
    reqObj["signal"] = boost::json::array{boost::json::array{-88.65925598144531, -65.49491882324219}};
    std::string jsonStr = boost::json::serialize(reqObj);
    std::vector<uint8_t> payload(jsonStr.begin(), jsonStr.end());
    TransportMessage request("signal", Transaction::Request, payload);

    auto response = sendAndReceive(*clientHandler, request);
    EXPECT_EQ(response.type, "signal");
    EXPECT_EQ(response.transaction, Transaction::Response);


    try {
        auto jv = boost::json::parse(std::string(response.payload.begin(), response.payload.end()));
        EXPECT_EQ(jv.at("type").as_string(), "signal");
        EXPECT_EQ(jv.at("transaction").as_int64(), static_cast<int>(Transaction::Response));
        EXPECT_EQ(jv.at("central_Freq").as_int64(), 6100);
        auto signalArr = jv.at("signal").as_array();
        EXPECT_EQ(signalArr.size(), 1);
        auto point = signalArr[0].as_array();
        EXPECT_DOUBLE_EQ(point[0].as_double(), -88.65925598144531);
        EXPECT_DOUBLE_EQ(point[1].as_double(), -65.49491882324219);
    } catch (const std::exception &e) {
        FAIL() << "JSON parse error: " << e.what();
    }
}

TEST_F(SegmentServerTest, InformationRequest_ReturnsInformationResponse) {
    startServer();
    auto clientHandler = connectClient();
    ASSERT_NE(clientHandler, nullptr);

    boost::json::object reqObj;
    reqObj["type"] = "information";
    reqObj["transaction"] = 1;
    reqObj["numberCore"] = 4;
    std::string jsonStr = boost::json::serialize(reqObj);
    std::vector<uint8_t> payload(jsonStr.begin(), jsonStr.end());
    TransportMessage request("information", Transaction::Request, payload);

    auto response = sendAndReceive(*clientHandler, request);
    EXPECT_EQ(response.type, "information");
    EXPECT_EQ(response.transaction, Transaction::Response);

    auto jv = boost::json::parse(std::string(response.payload.begin(), response.payload.end()));
    EXPECT_EQ(jv.at("type").as_string(), "information");
    EXPECT_EQ(jv.at("numberCore").as_int64(), 4);
}

TEST_F(SegmentServerTest, MultiConnectDisabled_OnlyOneConnectionAllowed) {
    startServer(false);

    auto client1 = connectClient();
    ASSERT_NE(client1, nullptr);

    // Формируем правильный JSON для information запроса
    boost::json::object reqObj;
    reqObj["type"] = "information";
    reqObj["transaction"] = static_cast<int>(Transaction::Request);
    reqObj["numberCore"] = 4;
    std::string jsonStr = boost::json::serialize(reqObj);
    std::vector<uint8_t> payload(jsonStr.begin(), jsonStr.end());
    TransportMessage req("information", Transaction::Request, payload);

    auto resp1 = sendAndReceive(*client1, req);
    EXPECT_EQ(resp1.type, "information");

    auto client2 = connectClient();
    if (client2) {
        auto resp2 = sendAndReceive(*client2, req);
        EXPECT_EQ(resp2.transaction, Transaction::Error);
    } else {
        SUCCEED();
    }
}

// + TODO: убедиться, что не создаем больше потоков, чем есть сокетов в тесте
/// (отследить по логам либо (рекомендуется) по счетчиу вызова callback
TEST_F(SegmentServerTest, ChechCounterThread) {
    bool isMultiConnect = false;
    short unsigned NClient = 100;
    startServer(isMultiConnect);
    std::vector<std::shared_ptr<ConnectionHandler> > clients;
    for (int i = 0; i < NClient; i++) {
        auto client = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Client, true);
        client->start();
        client->connect();
        clients.push_back(client);
    }

    if (isMultiConnect) {
        while (server->getConnectedClients() != NClient)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        EXPECT_EQ(server->getAliveThreads(), 2+NClient);
    } else {
        while (server->getConnectedClients() != 1)
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

    auto clientHandler = connectClient();
    ASSERT_NE(clientHandler, nullptr);

    Network::ConnectionId clientId = 0;
    server->setNewConnectionHandler([&clientId](Network::ConnectionId id) {
        clientId = id;
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_EQ(clientId, 0);

    boost::json::object reqObj;
    reqObj["type"] = "information";
    reqObj["transaction"] = static_cast<int>(Transaction::Request);
    reqObj["numberCore"] = 4;
    std::string jsonStr = boost::json::serialize(reqObj);
    std::vector<uint8_t> payload(jsonStr.begin(), jsonStr.end());
    TransportMessage request("information", Transaction::Request, payload);

    const int numMessages = 10;

    const int numMessagesBeforeDisconnect = numMessages/2;

    for (int i = 0; i < numMessages; ++i) {
        if (i <= numMessagesBeforeDisconnect)
            ASSERT_TRUE(clientHandler->write(request));
        else
            ASSERT_FALSE(clientHandler->write(request));
        if (i == numMessagesBeforeDisconnect) {
            server->disconnect(clientId);
            // break;
        }
    }


    int waitCount = 0;
    while (server->getAliveThreads() != 2 && waitCount < 50) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        ++waitCount;
    }


    EXPECT_EQ(server->getAliveThreads(), 2);

    EXPECT_TRUE(server->isRunning());

    auto newClient = connectClient();
    ASSERT_NE(newClient, nullptr);

    auto response = sendAndReceive(*newClient, request);
    EXPECT_EQ(response.type, "information");
    EXPECT_EQ(response.transaction, Transaction::Response);
}