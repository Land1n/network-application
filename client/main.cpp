// //
// // Created by ivan on 14.03.2026.
// //
//
// #include "SegmentClient.hpp"
// #include <iostream>
//
// #include "clientserveriface/connectionid.h"
//
// std::atomic<bool> running{true};
//
// void signalHandler(int signum) {
//     std::cout << "\nInterrupt signal (" << signum << ") received. Shutting down...\n";
//     running = false;
// }
//
// int main() {
//     std::string serverAddress = "127.0.0.1";
//     int serverPort = 8080;
//     bool debug = false;
//
//     std::signal(SIGINT, signalHandler);
//     std::signal(SIGTERM, signalHandler);
//
//     SegmentClient client(serverAddress, serverPort, debug);
//
//     // client.setNewConnectionHandler([]() {
//     //     std::cout << "[Client] Connected to server" << std::endl;
//     // });
//     //
//     // client.setCloseConnectionHandler([]() {
//     //     std::cout << "[Client] Connection closed" << std::endl;
//     // });
//     client.setReadHandler([](const void* data,size_t sz) {
//         const unsigned char *bytes = (const unsigned char *) data;
//         for (int i = 0; i < sz; i++) {
//             std::cout << bytes[i];
//         }
//         std::cout << std::endl;
//
//     });
//     client.start();
//     client.connect();
//
//     const char* msg = "Hello Server!";
//     client.write(msg, strlen(msg));
//     std::cout << "[Client] Sent: " << msg << std::endl;
//
//     while (running) {
//         std::this_thread::sleep_for(std::chrono::milliseconds(10));
//         if (!client.isRunning()) break;
//     }
//
//     client.disconnect();
//     client.stop();
//     return 0;
// }
#include "SegmentClientCreator.hpp"
#include "clientserveriface/client.h"
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <cstring>

std::atomic<bool> running{true};

void signal_handler(int) {
    running = false;
}

int main() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    Network::ClientCreatorParams params;
    params.hostname = "127.0.0.1";
    params.port = 8080;
    params.logLevel = static_cast<uint32_t>(LogLevel::Debug);

    SegmentClientCreator creator;
    auto net_client = creator.create(params);
    auto* client = reinterpret_cast<SegmentClient*>(net_client.get());
    client->setReadHandler([](const void* data, size_t sz) {
        std::cout.write(static_cast<const char*>(data), sz) << std::endl;
    });

    client->start();
    client->connect();


    const char* msg = "Hello from creator!";
    client->write(msg, std::strlen(msg));

    while (running && client) {
        if (!client->isRunning()) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    client->disconnect();
    client->stop();
    return 0;
}