//
// Created by ivan on 14.03.2026.
//

#include "SegmentClient.hpp"
#include <iostream>

int main() {
    // SegmentClient client("127.0.0.1", 8080, true, 5000); // reconnect after 5 sec
    // client.setNewConnectionHandler([]() {
    //     std::cout << "Connected to server" << std::endl;
    // });
    // client.setReadHandler([](const void* data, size_t sz) {
    //     std::string msg(static_cast<const char*>(data), sz);
    //     std::cout << "Received: " << msg << std::endl;
    // });
    // client.setCloseConnectionHandler([]() {
    //     std::cout << "Connection closed" << std::endl;
    // });
    //
    // client.start();
    //
    // // Отправить сообщение
    // std::string msg = "Hello, server!";
    // client.write(msg.data(), msg.size());
    //
    // std::this_thread::sleep_for(std::chrono::seconds(10));
    // client.stop();
    // return 0;
}