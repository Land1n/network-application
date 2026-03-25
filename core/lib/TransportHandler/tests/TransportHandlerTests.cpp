//
// Created by ivan on 07.03.2026.
//

#include "ConnectionHandler.hpp"
#include "TransportHandler.hpp"

#include <boost/asio.hpp>

#include <gtest/gtest.h>
#include <thread>

GTEST_TEST(TransportHandlerTest, SendAndReadOnSocketTest) {
    auto io_context_server = std::make_shared<boost::asio::io_context>();
    auto io_context_client = std::make_shared<boost::asio::io_context>();
    auto is_working = std::make_shared<std::atomic<bool>>(true);

    std::string address = "127.0.0.1";
    int port = 1234;

    ConnectionHandler server_handler(address, port, io_context_server,true);
    ConnectionHandler client_handler(address, port, io_context_client,true);

    auto acceptor = server_handler.listen(is_working);
    ASSERT_NE(acceptor, nullptr);

    std::vector<uint8_t> test_data = {1, 2, 3, 4, 5};

    std::thread server_thread([&server_handler, acceptor, &test_data]() {
        auto socket_on_server = server_handler.accept(acceptor);
        ASSERT_NE(socket_on_server, nullptr);
        TransportHandler server_transport(socket_on_server);
        TransportMessage msg = server_transport.read();
        EXPECT_EQ(msg.payload, test_data);
    });

    std::thread client_thread([&client_handler, &test_data]() {
        auto socket_on_client = client_handler.connect();
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