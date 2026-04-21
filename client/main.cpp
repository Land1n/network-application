//
// Created by ivan on 14.03.2026.
//

#include "SegmentClient.hpp"
#include <iostream>

#include "clientserveriface/connectionid.h"

std::atomic<bool> running{true};

void signalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received. Shutting down...\n";
    running = false;
}

int main() {
    std::string serverAddress = "127.0.0.1";
    int serverPort = 8080;
    bool debug = false;

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    SegmentClient client(serverAddress, serverPort, debug);

    // client.setNewConnectionHandler([]() {
    //     std::cout << "[Client] Connected to server" << std::endl;
    // });
    //
    // client.setCloseConnectionHandler([]() {
    //     std::cout << "[Client] Connection closed" << std::endl;
    // });
    client.setReadHandler([](const void* data,size_t sz) {
        const unsigned char *bytes = (const unsigned char *) data;
        for (int i = 0; i < sz; i++) {
            std::cout << bytes[i];
        }
        std::cout << std::endl;

    });
    client.start();
    client.connect();

    // const char* msg = "Hello Server!";
    // client.write(msg, strlen(msg));
    // std::cout << "[Client] Sent: " << msg << std::endl;

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (!client.isRunning()) break;
    }

    client.disconnect();
    return 0;
}