//
// Created by ivan on 07.03.2026.
//

#include "ConnectionHandler.hpp"
#include <gtest/gtest.h>


GTEST_TEST(ConnectionHandlerTests, ConnectionHandlerServerAddressInUse) {

    std::shared_ptr<std::atomic<bool>> is_working_1 = std::make_shared<std::atomic<bool>>(true);
    std::shared_ptr<std::atomic<bool>> is_working_2 = std::make_shared<std::atomic<bool>>(true);

    auto context_server_1 = std::make_shared<boost::asio::io_context>();
    auto context_server_2 = std::make_shared<boost::asio::io_context>();
    std::string address = "127.0.0.1";
    int port = 1234;

    ConnectionHandler server_handler_1(address,port,context_server_1,true);
    ConnectionHandler server_handler_2(address,port,context_server_2,true);
    std::shared_ptr<tcp::acceptor> acceptor_1 = server_handler_1.listen(is_working_1);
    std::shared_ptr<tcp::acceptor> acceptor_2 = server_handler_2.listen(is_working_2);
    EXPECT_NE(acceptor_1,nullptr);
    EXPECT_TRUE(is_working_1->load());
    EXPECT_NE(acceptor_2,nullptr);
    EXPECT_TRUE(is_working_2->load());
}

GTEST_TEST(ConnectionHandlerTests, ConnectionHandlerServerAcceptClientTest) {
    std::shared_ptr<std::atomic<bool>> is_working = std::make_shared<std::atomic<bool>>(true);

    auto context_server = std::make_shared<boost::asio::io_context>();
    auto context_client = std::make_shared<boost::asio::io_context>();

    std::string address = "127.0.0.1";
    int port = 1234;

    ConnectionHandler server_handler(address,port,context_server,true);
    std::shared_ptr<tcp::acceptor> acceptor = server_handler.listen(is_working);

    EXPECT_NE(acceptor,nullptr);
    EXPECT_TRUE(is_working->load());

    ConnectionHandler client_handler(address,port,context_client,true);

    std::thread server_thread([&server_handler, acceptor]() {
        std::shared_ptr<tcp::socket> socket_on_server = server_handler.accept(acceptor);
        EXPECT_NE(socket_on_server, nullptr);
    });

    std::thread client_thread([&client_handler]() {
        std::shared_ptr<tcp::socket> socket_on_client = client_handler.connect();
        EXPECT_NE(socket_on_client, nullptr);
    });

    server_thread.join();
    client_thread.join();
}
