//
// Created by ivan on 07.03.2026.
//

#include "ConnectionHandler.hpp"
#include <gtest/gtest.h>

#include <chrono>
#include <condition_variable>

GTEST_TEST(ConnectionHandlerTest, StartStop) {
    std::string address = "127.0.0.1";
    ConnectionHandler server(address, 8000, ConnectionHandlerType::Server,false);
    server.start();
    EXPECT_NE(server.getAcceptor(), nullptr);
    server.stop();
    EXPECT_EQ(server.getAcceptor(), nullptr);
}

GTEST_TEST(ConnectionHandlerTest, CreateAcceptorClientFails) {
    std::string address = "127.0.0.1";
    ConnectionHandler client(address, 8080, ConnectionHandlerType::Client,false);
    client.start();
    EXPECT_EQ(client.getAcceptor(), nullptr);
}

GTEST_TEST(ConnectionHandlerLifecycleTest, StartStopBehavior) {
    int port = 8080;
    std::string address = "127.0.0.1";

    auto handler = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Server,false);
    handler->start();
    EXPECT_TRUE(handler->isInWork());

    handler->stop();
    EXPECT_FALSE(handler->isInWork());

    handler->stop();
    EXPECT_FALSE(handler->isInWork());

    auto sock = handler->accept();
    EXPECT_EQ(sock, nullptr);
}
/*
 * Подключается клиент, сервер отслеживает подключение
 * и как только зачислил подключение, клиент отключается
 */
GTEST_TEST(ConnectionHandlerClientTest, ConnectAndDisconnect) {
    int port = 8080;
    std::string address = "127.0.0.1";
    auto server = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Server,false);
    auto client = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Client,false);

    server->start();
    client->start();

    std::shared_ptr<std::mutex> mutex = std::make_shared<std::mutex>();
    client->setLoggerMutex(mutex);
    server->setLoggerMutex(mutex);  // общий логгер для теста

    server->listen();
    auto client_sock = client->connect();
    ASSERT_NE(client_sock, nullptr);
    auto start = std::chrono::steady_clock::now();
    while (server->getSockets().empty()) {
        if (std::chrono::steady_clock::now() - start > std::chrono::seconds(1)) {
            FAIL() << "Timeout waiting for server";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    ASSERT_EQ(server->getSockets().size(),1);

    client->disconnect();
    start = std::chrono::steady_clock::now();

    while (!server->getSockets().empty()) {
        if (std::chrono::steady_clock::now() - start > std::chrono::seconds(1)) {
            FAIL() << "Timeout waiting for server";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    ASSERT_EQ(server->getSockets().size(),0);
    client->stop();
    server->stop();
}
/*
 *  Подключаются двое и после этого сервер сразу выключается
 */
GTEST_TEST(ConnectionHandlerServerTest, TwoConnectAndStopServer) {
    std::string address = "127.0.0.1";
    int port = 8080;
    auto server = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Server);
    server->start();

    auto mutex = std::make_shared<std::mutex>();
    server->setLoggerMutex(mutex);

    server->listen();
    std::vector<std::shared_ptr<ConnectionHandler>> clients;
    for (int i = 0; i < 2; i++) {
        auto c = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Client);
        c->start();
        c->setLoggerMutex(mutex);
        auto client_sock = c->connect();
        clients.push_back(c);
        ASSERT_NE(client_sock, nullptr);
    }
    auto start = std::chrono::steady_clock::now();

    while (server->getSockets().size() != 2) {
        if (std::chrono::steady_clock::now() - start > std::chrono::seconds(1)) {
            FAIL() << "Timeout waiting for server";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    server->stop();
    EXPECT_EQ(server->getAcceptor(),nullptr);
    EXPECT_EQ(server->getSockets().size(),0);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    for (auto& c : clients) {
        auto sock = c->getSocket();
        ASSERT_NE(sock, nullptr);
        EXPECT_FALSE(sock->is_open());
    }
}
