//
// Created by ivan on 07.03.2026.
//

#include "ConnectionHandler.hpp"
#include "TransportHandler.hpp"

#include <boost/asio.hpp>

#include <gtest/gtest.h>
#include <thread>

#include "ConnectionHandler.hpp"
#include "TransportHandler.hpp"
#include <boost/asio.hpp>
#include <gtest/gtest.h>
#include <thread>

GTEST_TEST(TransportHandlerTest, SendAndReadOnSocketTest) {
    std::string address = "127.0.0.1";
    int port = 8080;

    // No thread pool – we manage threads manually in this test
    auto server_handler = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Server);
    auto client_handler = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Client);

    auto acceptor = server_handler->getAcceptor();
    ASSERT_NE(acceptor, nullptr);
    acceptor->listen();

    std::vector<uint8_t> test_data = {1, 2, 3, 4, 8};

    std::thread server_thread([server_handler, &test_data]() {
        auto socket_on_server = server_handler->accept();
        ASSERT_NE(socket_on_server, nullptr);
        TransportHandler server_transport(socket_on_server);
        TransportMessage msg = server_transport.read();
        EXPECT_EQ(msg.payload, test_data);
    });

    std::thread client_thread([client_handler, &test_data]() {
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));
        auto socket_on_client = client_handler->connect();
        ASSERT_NE(socket_on_client, nullptr);
        TransportHandler client_transport(socket_on_client);
        TransportMessage msg;
        msg.payload = test_data;
        bool sent = client_transport.send(msg);
        EXPECT_TRUE(sent);
    });

    server_thread.join();
    client_thread.join();
}
