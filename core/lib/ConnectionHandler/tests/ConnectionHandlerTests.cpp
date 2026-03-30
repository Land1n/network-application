// //
// // Created by ivan on 07.03.2026.
// //
//
// #include "ConnectionHandler.hpp"
// #include <gtest/gtest.h>
//
// #include "ThreadPool.hpp"
//
// GTEST_TEST(ConnectionHandlerTest, StartStop) {
//     ConnectionHandler server(8000, ConnectionHandlerType::Server, true);
//     EXPECT_NE(server.getAcceptor(), nullptr);
//     server.stop();
//     EXPECT_EQ(server.getAcceptor(), nullptr);
// }
//
// GTEST_TEST(ConnectionHandlerTest, CreateAcceptorClientFails) {
//     ConnectionHandler client(8080, ConnectionHandlerType::Client);
//     EXPECT_EQ(client.getAcceptor(), nullptr);
// }
//
// GTEST_TEST(ConnectionHandlerTest, AcceptSingleConnection) {
//     ConnectionHandler server(8000, ConnectionHandlerType::Server);
//     auto acceptor = server.getAcceptor();
//     ASSERT_NE(acceptor, nullptr);
//     acceptor->listen();
//
//     std::thread accept_thread([&server ]() {
//         auto sock = server.accept();
//         EXPECT_NE(sock, nullptr);
//     });
//
//     ConnectionHandler client(8000, ConnectionHandlerType::Client);
//
//     std::thread connect_thread([&client ]() {
//         auto sock = client.connect();
//         EXPECT_NE(sock, nullptr);
//     });
//     accept_thread.join();
//     connect_thread.join();
//     auto client_sock = client.getSocket();
//     ASSERT_NE(client_sock, nullptr);
//     auto &sockets = server.getSockets();
//     EXPECT_EQ(sockets.size(), 1);
//     EXPECT_EQ(sockets[0].port, client_sock->local_endpoint().port());
//     EXPECT_EQ(sockets[0].address, "127.0.0.1");
// }
// //
// // GTEST_TEST(ConnectionHandlerTest, DisconnectedClosesSocket) {
// //     ConnectionHandler server(8000, ConnectionHandlerType::Server);
// //     auto acceptor = server.getAcceptor();
// //     ASSERT_NE(acceptor, nullptr);
// //     acceptor->listen();
// //
// //     std::thread accept_thread([&server ]() {
// //         auto sock = server.accept();
// //         EXPECT_NE(sock, nullptr);
// //     });
// //
// //     ConnectionHandler client(8000, ConnectionHandlerType::Client);
// //
// //     std::thread connect_thread([&client ]() {
// //         auto sock = client.connect();
// //         EXPECT_NE(sock, nullptr);
// //         client.disconnect();
// //     });
// //     accept_thread.join();
// //     connect_thread.join();
// //
//
//
//     // ConnectionHandler server(8000, ConnectionHandlerType::Server);
//     // auto acceptor = server.getAcceptor();
//     // ASSERT_NE(acceptor, nullptr);
//     // acceptor->listen();
//     //
//     // std::thread accept_thread([&server]() {
//     //     auto sock = server.accept();
//     //     EXPECT_NE(sock, nullptr);
//     // });
//     //
//     // ConnectionHandler client(8000, ConnectionHandlerType::Client);
//     // accept_thread.join();
//     // auto client_sock = client.connect();
//     // ASSERT_NE(client_sock, nullptr);
//     //
//     // auto& sockets = server.getSockets();
//     // ASSERT_EQ(sockets.size(), 1);
//     // ConnectedSocket& conn = sockets[0];
//     //
//     // // Проверяем, что соединение живое
//     // boost::system::error_code ec;
//     // client_sock->write_some(boost::asio::buffer("ping"), ec);
//     // EXPECT_FALSE(ec);
//     //
//     // // Закрываем со стороны сервера
//     // bool result = server.disconnected(conn);
//     // EXPECT_TRUE(result);
//     //
//     // // Повторная запись должна завершиться ошибкой
//     // client_sock->write_some(boost::asio::buffer("ping"), ec);
//     // EXPECT_TRUE(ec);
// // }