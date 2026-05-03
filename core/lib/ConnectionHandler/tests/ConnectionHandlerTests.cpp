//
// Created by ivan on 07.03.2026.
//

#include "ConnectionHandler.hpp"
#include <gtest/gtest.h>

#include <chrono>
#include <condition_variable>


class ConnectionHandlerTests : public ::testing::Test {
public:
    std::string address = "127.0.0.1";
    int port = 8080;
    std::shared_ptr<ConnectionHandler> server;
    std::shared_ptr<ConnectionHandler> client;

    void SetUp() override {
        server = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Server);
        client = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Client);
        Logger::getInstance().setLevel(LogLevel::Tests);
    };
    void TearDown() override {
        if (server) server->stop();
        if (client) client->stop();
    }
};

TEST_F(ConnectionHandlerTests, ServerStartStop) {
    server->start();
    EXPECT_NE(server->getAcceptor(), nullptr);
    server->stop();
    EXPECT_EQ(server->getAcceptor(), nullptr);
}

TEST_F(ConnectionHandlerTests, ClientFailsCreateAcceptor) {
    client->start();
    EXPECT_EQ(client->getAcceptor(), nullptr);
}

TEST_F(ConnectionHandlerTests, StartStopBehavior) {
    server->start();
    EXPECT_TRUE(server->getIsWork());

    server->stop();
    EXPECT_FALSE(server->getIsWork());

    server->stop();
    EXPECT_FALSE(server->getIsWork());

    auto sock = server->accept();
    EXPECT_EQ(sock.ptr, nullptr);
}

/*
 * Подключается клиент, сервер отслеживает подключение
 * и как только зачислил подключение, клиент отключается
 */
TEST_F(ConnectionHandlerTests, ConnectAndDisconnect) {
    server->start();
    client->start();

    server->listen();
    auto client_sock = client->connect();
    ASSERT_NE(client_sock.ptr, nullptr);
    auto start = std::chrono::steady_clock::now();
    while (server->getSockets().empty()) {
        if (std::chrono::steady_clock::now() - start > std::chrono::seconds(1)) {
            FAIL() << "Timeout waiting for server";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    ASSERT_EQ(server->getSockets().size(), 1);

    client->disconnect();
    start = std::chrono::steady_clock::now();

    while (!server->getSockets().empty()) {
        if (std::chrono::steady_clock::now() - start > std::chrono::seconds(1)) {
            FAIL() << "Timeout waiting for server";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    ASSERT_EQ(server->getSockets().size(), 0);
}

/*
 * Тест на подключение к серверу клиента с неправильным таском
 */
TEST_F(ConnectionHandlerTests, BadClientTask) {
    server->start();
    client->start();

    server->listen();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    client->setTaskSocket([](ConnectedSocket &) {
        throw std::runtime_error("Test exception from task");
    });
    client->connect();
    ASSERT_NE(client, nullptr);
    ASSERT_NE(client->getSocket().ptr, nullptr);
}

TEST_F(ConnectionHandlerTests, BadServerTask) {
    server->setTaskSocket([](ConnectedSocket &) {
        throw std::runtime_error("Test exception from server task");
    });
    server->start();
    server->listen();
    client->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto client_sock = client->connect();
    ASSERT_NE(client_sock.ptr, nullptr);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_TRUE(server->getIsWork());
    EXPECT_EQ(server->getSockets().size(), 1);

    auto client2 = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Client, true);
    client2->start();
    auto client_sock2 = client2->connect();
    ASSERT_NE(client_sock2.ptr, nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(server->getSockets().size(), 2);

    client->disconnect();
    client2->disconnect();
}

// Тест на несколько клиентов с разными задачами
TEST_F(ConnectionHandlerTests, MultipleClientsWithTasks) {
    server->start();

    std::atomic<int> task_counter{0};
    server->setTaskSocket([&task_counter](ConnectedSocket &) {
        task_counter++;
    });

    server->listen();
    const int N = 5;
    std::vector<std::shared_ptr<ConnectionHandler> > clients;

    for (int i = 0; i < N; ++i) {
        auto client = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Client, true);
        client->start();
        client->connect();
        clients.push_back(client);
    }
    auto start = std::chrono::steady_clock::now();
    while (server->getSockets().size() != N) {
        if (std::chrono::steady_clock::now() - start > std::chrono::seconds(1 + N / 10)) {
            FAIL() << "Timeout waiting for server";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_EQ(server->getSockets().size(), N);
    EXPECT_EQ(task_counter.load(), N);

    for (auto &c: clients) c->stop();
}

// Тест на изменение Task
TEST_F(ConnectionHandlerTests, ChangeTaskAfterListen) {
    server->start();

    std::atomic<int> first_task_counter{0};
    server->setTaskSocket([&first_task_counter](ConnectedSocket &) { first_task_counter++; });
    server->listen();

    // Подключаем первого клиента
    auto client1 = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Client, true);
    client1->start();
    client1->connect();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(first_task_counter.load(), 1);

    // Меняем задачу
    std::atomic<int> second_task_counter{0};
    server->setTaskSocket([&second_task_counter](ConnectedSocket &) { second_task_counter++; });

    // Подключаем второго клиента
    auto client2 = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Client, true);
    client2->start();
    client2->connect();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_EQ(first_task_counter.load(), 1); // первая задача не вызывалась для второго клиента
    EXPECT_EQ(second_task_counter.load(), 1);

    client1->stop();
    client2->stop();
}

/*
 *  Стресс-тест подключаются N и после этого сервер сразу выключается
 */
TEST_F(ConnectionHandlerTests, NConnectAndStopServer) {
    int N = 100;
    server->start();


    server->listen();
    std::vector<std::shared_ptr<ConnectionHandler> > clients;
    for (int i = 0; i < N; i++) {
        auto c = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Client, true);
        c->start();
        auto client_sock = c->connect();
        clients.push_back(c);
        ASSERT_NE(client_sock.ptr, nullptr);
    }
    auto start = std::chrono::steady_clock::now();

    while (server->getSockets().size() != N) {
        if (std::chrono::steady_clock::now() - start > std::chrono::seconds(1 + N / 10)) {
            FAIL() << "Timeout waiting for server";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    server->stop();
    EXPECT_EQ(server->getAcceptor(), nullptr);
    EXPECT_EQ(server->getSockets().size(), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    for (auto &c: clients) {
        auto sock = c->getSocket();
        ASSERT_NE(sock.ptr, nullptr);
        EXPECT_FALSE(sock.ptr->is_open());
        c->stop();
    }
}

// Стресс-тест: долгоживущие соединения (100 клиентов на 5 секунд)
TEST_F(ConnectionHandlerTests, LongLivedConnections) {
    const int num_clients = 100;
    const int duration_seconds = 5;

    server->start();
    server->listen();

    std::vector<std::shared_ptr<ConnectionHandler> > clients;
    for (int i = 0; i < num_clients; ++i) {
        auto client = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Client, true);
        client->start();
        auto sock = client->connect();
        ASSERT_NE(sock.ptr, nullptr);
        clients.push_back(client);
    }

    auto start_time = std::chrono::steady_clock::now();
    while (server->getSockets().size() < num_clients) {
        if (std::chrono::steady_clock::now() - start_time > std::chrono::seconds(30)) {
            FAIL() << "Timeout waiting for clients to connect";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_EQ(server->getSockets().size(), num_clients);

    std::this_thread::sleep_for(std::chrono::seconds(duration_seconds));

    EXPECT_EQ(server->getSockets().size(), num_clients);

    for (auto &client: clients) {
        client->disconnect();
        client->stop();
    }

    while (!server->getSockets().empty()) {
        if (std::chrono::steady_clock::now() - start_time > std::chrono::seconds(30 + duration_seconds)) {
            FAIL() << "Timeout waiting for server to clear sockets";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_TRUE(server->getSockets().empty());
}
