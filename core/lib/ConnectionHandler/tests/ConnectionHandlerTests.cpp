//
// Created by ivan on 07.03.2026.
//

#include "ConnectionHandler.hpp"
#include <gtest/gtest.h>

GTEST_TEST(ConnectionHandlerTest, StartStop) {
    std::string address = "127.0.0.1";
    ConnectionHandler server(address, 8000, ConnectionHandlerType::Server);
    EXPECT_NE(server.getAcceptor(), nullptr);
    server.stop();
    EXPECT_EQ(server.getAcceptor(), nullptr);
}

GTEST_TEST(ConnectionHandlerTest, CreateAcceptorClientFails) {
    std::string address = "127.0.0.1";
    ConnectionHandler client(address, 8080, ConnectionHandlerType::Client);
    EXPECT_EQ(client.getAcceptor(), nullptr);
}

GTEST_TEST(ConnectionHandlerLifecycleTest, StartStopBehavior) {
    int port = 8080;
    std::string address = "127.0.0.1";

    auto handler = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Server,true);
    EXPECT_TRUE(handler->isInWork());

    handler->stop();
    EXPECT_FALSE(handler->isInWork());

    handler->stop();
    EXPECT_FALSE(handler->isInWork());

    auto sock = handler->accept();
    EXPECT_EQ(sock, nullptr);
}

GTEST_TEST(ConnectionHandlerClientTest, ConnectAndDisconnect) {
    int port = 8080;
    std::string address = "127.0.0.1";

    std::thread server_thread([&]() {
        auto server = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Server);
        server->listen();
        auto &server_sockets = server->getSockets();
        while (server_sockets.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        ASSERT_EQ(server_sockets.size(), 1);
        EXPECT_NE(server_sockets[0].ptr, nullptr);
        while (!server_sockets.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        ASSERT_EQ(server_sockets.size(), 0);
        EXPECT_EQ(server_sockets[0].ptr, nullptr);
        server->stop();
    });
    std::thread client_thread([&]() {
        auto client = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Client);
        client->connect();
        client->disconnect();
        client->stop();
    });

    server_thread.join();
    client_thread.join();

}

GTEST_TEST(ConnectionHandlerServerTest, ListenAndAcceptOneClient) {
    int port = 8080;
    std::string address = "127.0.0.1";
    auto server = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Server);
    server->listen();
    auto client = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Client);
    auto client_sock = client->connect();
    ASSERT_NE(client_sock, nullptr);

    auto &sockets = server->getSockets();
    ASSERT_EQ(sockets.size(), 1);
    EXPECT_NE(sockets[0].ptr, nullptr);
    EXPECT_EQ(sockets[0].address, address);
    EXPECT_GT(sockets[0].port, 0);

    client->stop();
    server->stop();
}
