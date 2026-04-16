#include "SegmentServer.hpp"
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>

static std::atomic<bool> running{true};

void signalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received. Shutting down...\n";
    running = false;
}
int main() {

    std::string address = "127.0.0.1";
    int port = 8080;
    bool debug = false;

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    SegmentServer server(address, port, debug);

    server.setNewConnectionHandler([](Network::ConnectionId id) {
        std::cout << "[Server] New connection: " << id << std::endl;
    });

    server.setCloseConnectionHandler([](Network::ConnectionId id) {
        std::cout << "[Server] Connection closed: " << id << std::endl;
    });

    server.setReadHandler([](Network::ConnectionId id, const void* data, size_t sz) {
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        std::cout << "[Server] Received " << sz << " bytes from " << id << ": ";
        for (size_t i = 0; i < sz && i < 100; ++i) {
            std::cout << std::hex << (int)bytes[i] << " ";
        }
        if (sz > 100) std::cout << "...";
        std::cout << std::dec << std::endl;
    });


    std::cout << "Starting server on " << address << ":" << port << std::endl;
    server.start();


    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "Stopping server..." << std::endl;
    server.stop();

    std::cout << "Server stopped. Goodbye." << std::endl;
    return 0;
}