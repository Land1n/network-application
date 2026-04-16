//
// Created by ivan on 14.03.2026.
//

#include "SegmentClient.hpp"
#include <iostream>

#include "clientserveriface/connectionid.h"

static std::atomic<bool> running{true};

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

    client.setNewConnectionHandler([]() {
        std::cout << "[Client] Connected to server" << std::endl;
    });

    client.setCloseConnectionHandler([]() {
        std::cout << "[Client] Connection closed" << std::endl;
    });

    std::cout << "Connecting to " << serverAddress << ":" << serverPort << "..." << std::endl;
    client.connect();

    std::thread sender([&client]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (running) {
            const char* msg = "Hello world!";
            client.write(msg, strlen(msg));
            std::cout << "[Client] Sent: " << msg << std::endl;
        }
    });

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    sender.join();
    client.disconnect();
    std::cout << "Client stopped." << std::endl;
    return 0;
}