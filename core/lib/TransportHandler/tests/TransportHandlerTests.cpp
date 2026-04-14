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
#include <boost/json.hpp>

namespace json = boost::json;

GTEST_TEST(TransportHandlerTest, SendAndReadOnSocketTest) {
    std::string address = "127.0.0.1";
    int port = 8080;

    auto server_handler = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Server, false);
    auto client_handler = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Client, false);
    server_handler->start();
    client_handler->start();
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
        bool sent = client_transport.write(msg);
        EXPECT_TRUE(sent);
    });

    server_thread.join();
    client_thread.join();
    server_handler->stop();
    client_handler->stop();
}

//Тест на несколько последовательных сообщений
GTEST_TEST(TransportHandlerTest, MultipleMessages) {
    std::string address = "127.0.0.1";
    int port = 8091;

    auto server = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Server, false);
    auto client = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Client, false);

    server->start();
    client->start();

    std::atomic<int> received_count{0};
    const int expected_count = 5;

    server->setTaskSocket([&](std::shared_ptr<tcp::socket> sock) {
        TransportHandler transport(sock);
        for (int i = 0; i < expected_count; ++i) {
            TransportMessage msg = transport.read();
            if (!msg.type.empty()) {
                received_count++;
            }
        }
    });
    server->listen();

    auto client_sock = client->connect();
    ASSERT_NE(client_sock, nullptr);
    TransportHandler client_transport(client_sock);

    for (int i = 0; i < expected_count; ++i) {
        boost::json::object obj;
        obj["type"] = "msg" + std::to_string(i);
        obj["transaction"] = 0;
        std::string json_str = boost::json::serialize(obj);
        TransportMessage msg;
        msg.type = "msg" + std::to_string(i);
        msg.transaction = Transaction::Request;
        msg.payload.assign(json_str.begin(), json_str.end());
        bool sent = client_transport.write(msg);
        EXPECT_TRUE(sent);
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(received_count.load(), expected_count);

    client->stop();
    server->stop();
}

//Тест на обрыв соединения во время чтения
GTEST_TEST(TransportHandlerTest, ConnectionLostDuringRead) {
    std::string address = "127.0.0.1";
    const int port = 8093;

    auto server = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Server, false);
    auto client = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Client, false);

    server->start();
    client->start();

    std::promise<void> read_started;
    std::future<void> read_started_future = read_started.get_future();

    server->setTaskSocket([&](std::shared_ptr<tcp::socket> sock) {
        TransportHandler transport(sock);
        read_started.set_value();
        TransportMessage msg = transport.read();
        EXPECT_TRUE(msg.type.empty());
    });
    server->listen();

    auto client_sock = client->connect();
    ASSERT_NE(client_sock, nullptr);

    uint32_t magic = 0xA0ABA0A;
    boost::asio::write(*client_sock, boost::asio::buffer(&magic, 4));
    read_started_future.wait();

    client_sock->close();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    client->stop();
    server->stop();
}

// Стресс-тест: множество клиентов обмениваются сообщениями через TransportHandler

GTEST_TEST(TransportHandlerStressTest, ManyClientsMessaging) {
    std::string address = "127.0.0.1";
    int port = 8094;
    const int num_clients = 10;
    const int messages_per_client = 10;

    auto server = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Server,false);
    server->start();

    std::atomic<int> server_handlers_active{0};
    std::mutex server_handlers_mutex;
    std::condition_variable server_handlers_cv;

    server->setTaskSocket([&](std::shared_ptr<tcp::socket> sock) {
        server_handlers_active++;
        TransportHandler transport(sock, 0xA0ABA0A, false);
        try {
            while (true) {
                TransportMessage req = transport.read();
                if (req.type.empty()) break;

                if (req.type == "ping") {
                    std::string json_str(req.payload.begin(), req.payload.end());
                    boost::json::value jv = boost::json::parse(json_str);
                    std::string data = boost::json::value_to<std::string>(jv.at("data"));

                    // Формируем ответ
                    std::string response_data = "pong " + data;
                    boost::json::object resp_obj;
                    resp_obj["type"] = "pong";
                    resp_obj["transaction"] = static_cast<int>(Transaction::Response);
                    resp_obj["data"] = response_data;
                    std::string resp_json = boost::json::serialize(resp_obj);
                    std::vector<uint8_t> resp_payload(resp_json.begin(), resp_json.end());
                    TransportMessage resp("pong", Transaction::Response, resp_payload);
                    transport.write(resp);
                }
            }
        } catch (const std::exception&) {

        }
        server_handlers_active--;
        server_handlers_cv.notify_one();
    });

    server->listen();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::atomic<int> clients_finished{0};
    std::mutex finish_mutex;
    std::condition_variable finish_cv;
    std::vector<std::shared_ptr<ConnectionHandler>> clients;
    clients.reserve(num_clients);

    auto start_time = std::chrono::steady_clock::now();


    for (int i = 0; i < num_clients; ++i) {
        auto client = std::make_shared<ConnectionHandler>(address, port, ConnectionHandlerType::Client,false);
        client->start();

        client->setTaskSocket([messages_per_client, &clients_finished, &finish_cv](std::shared_ptr<tcp::socket> sock) {
            TransportHandler transport(sock, 0xA0ABA0A, false);
            try {
                for (int msg_id = 1; msg_id <= messages_per_client; ++msg_id) {
                    // Формируем JSON-запрос

                    boost::json::object req_obj;
                    req_obj["type"] = "ping";
                    req_obj["transaction"] = static_cast<int>(Transaction::Request);
                    req_obj["data"] = "ping " + std::to_string(msg_id);
                    std::string req_json = boost::json::serialize(req_obj);
                    std::vector<uint8_t> req_payload(req_json.begin(), req_json.end());
                    TransportMessage req("ping", Transaction::Request, req_payload);
                    if (!transport.write(req)) break;

                    // Читаем ответ
                    TransportMessage resp = transport.read();
                    if (resp.type != "pong") break;
                    std::string resp_json(resp.payload.begin(), resp.payload.end());
                    boost::json::value jv = boost::json::parse(resp_json);
                    std::string response_data = boost::json::value_to<std::string>(jv.at("data"));
                    std::string expected = "pong ping " + std::to_string(msg_id);
                    EXPECT_EQ(response_data, expected);
                }
                // Закрываем соединение, чтобы серверный обработчик вышел из цикла
                sock->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
                sock->close();
            } catch (const std::exception&) {
                // игнорируем
            }
            clients_finished++;
            finish_cv.notify_one();
        });

        auto sock = client->connect();
        ASSERT_NE(sock, nullptr);
        clients.push_back(client);
    }


    {
        std::unique_lock<std::mutex> lock(server_handlers_mutex);
        bool all_handlers_done = server_handlers_cv.wait_for(lock, std::chrono::seconds(10),
            [&]() { return server_handlers_active.load() == 0; });
        EXPECT_TRUE(all_handlers_done) << "Server handlers did not finish";
    }


    auto duration = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - start_time);
    std::cout << "ManyClientsMessaging completed with " << num_clients << " clients, "
              << messages_per_client << " messages each in " << duration.count() << " seconds\n";


    for (auto& c : clients) {
        c->stop();
    }
    server->stop();
}