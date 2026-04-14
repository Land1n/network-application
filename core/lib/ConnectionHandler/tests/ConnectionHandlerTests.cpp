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
 * Тест на подключение к серверу клиента с неправильным таском
 */
GTEST_TEST(ConnectionHandlerServerTest,BadClientTask) {
    std::string address = "127.0.0.1";
    int port = 8080;

    auto server = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Server);
    auto client = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Client);

    server->start();
    client->start();

    server->listen();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    client->setTaskSocket([] (std::shared_ptr<tcp::socket>) {
        throw std::runtime_error("Test exception from task");
    });
    client->connect();
    ASSERT_NE(client, nullptr);
    ASSERT_NE(client->getSocket().ptr, nullptr);
    client->stop();
    server->stop();
}

GTEST_TEST(ConnectionHandlerServerTest, BadServerTask) {
    std::string address = "127.0.0.1";
    int port = 8000;

    auto server = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Server, false);
    auto client = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Client, false);

    server->start();
    client->start();


    server->setTaskSocket([](std::shared_ptr<tcp::socket>) {
        throw std::runtime_error("Test exception from server task");
    });

    server->listen();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto client_sock = client->connect();
    ASSERT_NE(client_sock, nullptr);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_TRUE(server->isInWork());
    EXPECT_EQ(server->getSockets().size(), 1);

    auto client2 = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Client, false);
    client2->start();
    auto client_sock2 = client2->connect();
    ASSERT_NE(client_sock2, nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(server->getSockets().size(), 2);

    client->disconnect();
    client2->disconnect();
    server->stop();
    client->stop();
    client2->stop();
}
// Тест на повторное подключение клиента после разрыва
GTEST_TEST(ConnectionHandlerClientTest, ReconnectAfterDisconnect) {
    std::string address = "127.0.0.1";
    int port = 8082;
    auto server = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Server, false);
    auto client = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Client, false);

    server->start();
    client->start();
    server->listen();

    auto sock1 = client->connect();
    ASSERT_NE(sock1, nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(server->getSockets().size(), 1);

    client->disconnect();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(server->getSockets().size(), 0);

    auto sock2 = client->connect();
    ASSERT_NE(sock2, nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(server->getSockets().size(), 1);

    client->stop();
    server->stop();
}
// Тест на несколько клиентов с разными задачами
GTEST_TEST(ConnectionHandlerServerTest, MultipleClientsWithTasks) {
    std::string address = "127.0.0.1";
    int port = 8083;
    auto server = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Server, false);
    server->start();

    std::atomic<int> task_counter{0};
    server->setTaskSocket([&task_counter](std::shared_ptr<tcp::socket>) {
        task_counter++;
    });

    server->listen();
    const int N = 5;
    std::vector<std::shared_ptr<ConnectionHandler>> clients;

    for (int i = 0; i < N; ++i) {
        auto client = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Client, false);
        client->start();
        client->connect();
        clients.push_back(client);
    }
    auto start = std::chrono::steady_clock::now();
    while (server->getSockets().size() != N) {
        if (std::chrono::steady_clock::now() - start > std::chrono::seconds(1+N/10)) {
            FAIL() << "Timeout waiting for server";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_EQ(server->getSockets().size(), N);
    EXPECT_EQ(task_counter.load(), N);

    for (auto& c : clients) c->stop();
    server->stop();
}
// Тест на изменение Task
GTEST_TEST(ConnectionHandlerServerTest, ChangeTaskAfterListen) {
    std::string address = "127.0.0.1";
    int port = 8086;
    auto server = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Server, false);
    server->start();

    std::atomic<int> first_task_counter{0};
    server->setTaskSocket([&first_task_counter](auto) { first_task_counter++; });
    server->listen();

    // Подключаем первого клиента
    auto client1 = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Client, false);
    client1->start();
    client1->connect();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(first_task_counter.load(), 1);

    // Меняем задачу
    std::atomic<int> second_task_counter{0};
    server->setTaskSocket([&second_task_counter](auto) { second_task_counter++; });

    // Подключаем второго клиента
    auto client2 = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Client, false);
    client2->start();
    client2->connect();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_EQ(first_task_counter.load(), 1);  // первая задача не вызывалась для второго клиента
    EXPECT_EQ(second_task_counter.load(), 1);

    client1->stop();
    client2->stop();
    server->stop();
}

/*
 *  Стресс-тест подключаются N и после этого сервер сразу выключается
 */
GTEST_TEST(ConnectionHandlerStressTest, NConnectAndStopServer) {
    std::string address = "127.0.0.1";
    int port = 8080;
    int N = 100;
    auto server = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Server,true);
    server->start();


    server->listen();
    std::vector<std::shared_ptr<ConnectionHandler>> clients;
    for (int i = 0; i < N; i++) {
        auto c = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Client,true);
        c->start();
        auto client_sock = c->connect();
        clients.push_back(c);
        ASSERT_NE(client_sock, nullptr);
    }
    auto start = std::chrono::steady_clock::now();

    while (server->getSockets().size() != N) {
        if (std::chrono::steady_clock::now() - start > std::chrono::seconds(1+N/10)) {
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
        ASSERT_NE(sock.ptr, nullptr);
        EXPECT_FALSE(sock.ptr->is_open());
        c->stop();
    }

}

// Стресс-тест: долгоживущие соединения (100 клиентов на 5 секунд)
GTEST_TEST(ConnectionHandlerStressTest, LongLivedConnections) {
    std::string address = "127.0.0.1";
    int port = 8084;
    const int num_clients = 100;
    const int duration_seconds = 5;

    auto server = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Server, false);
    server->start();
    server->listen();

    std::vector<std::shared_ptr<ConnectionHandler>> clients;
    for (int i = 0; i < num_clients; ++i) {
        auto client = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Client, false);
        client->start();
        auto sock = client->connect();
        ASSERT_NE(sock, nullptr);
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

    for (auto& client : clients) {
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

    server->stop();
}