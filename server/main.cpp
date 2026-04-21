// #include "SegmentServer.hpp"
// #include <iostream>
// #include <csignal>
// #include <atomic>
// #include <thread>
// #include <chrono>
//
// static std::atomic<bool> running{true};
//
// void signalHandler(int signum) {
//     std::cout << "\nInterrupt signal (" << signum << ") received. Shutting down...\n";
//     running = false;
// }
// int main() {
//
//     std::string address = "127.0.0.1";
//     int port = 8080;
//     bool debug = false;
//
//     std::signal(SIGINT, signalHandler);
//     std::signal(SIGTERM, signalHandler);
//
//     SegmentServer server(address, port, debug);
//
//     server.setNewConnectionHandler([](Network::ConnectionId id) {
//         std::cout << "[Server] New connection: " << id << std::endl;
//     });
//
//     server.setCloseConnectionHandler([](Network::ConnectionId id) {
//         std::cout << "[Server] Connection closed: " << id << std::endl;
//     });
//
//     server.setReadHandler([](Network::ConnectionId id, const void* data, size_t sz) {
//         const uint8_t* bytes = static_cast<const uint8_t*>(data);
//         std::cout << "[Server] Received " << sz << " bytes from " << id << ": ";
//         for (size_t i = 0; i < sz && i < 100; ++i) {
//             std::cout << std::hex << (int)bytes[i] << " ";
//         }
//         if (sz > 100) std::cout << "...";
//         std::cout << std::dec << std::endl;
//     });
//
//
//     std::cout << "Starting server on " << address << ":" << port << std::endl;
//     server.start();
//
//
//     while (running) {
//         std::this_thread::sleep_for(std::chrono::milliseconds(100));
//     }
//
//     std::cout << "Stopping server..." << std::endl;
//     server.stop();
//
//     std::cout << "Server stopped. Goodbye." << std::endl;
//     return 0;
// }

#include "ConnectionHandler.hpp"
#include <iostream>
#include <memory>
#include <csignal>
#include <atomic>

#include "SegmentServer.hpp"

// std::shared_ptr<ConnectionHandler> server;
// std::atomic<bool> shutdown_flag{false};
//
// void signal_handler(int sig) {
//     if (sig == SIGINT || sig == SIGTERM) {
//         std::cout << "\nInterrupt signal received. Shutting down..." << std::endl;
//         if (server) server->stop();
//         shutdown_flag = true;
//     }
// }
std::atomic<bool> running(true);

int main() {
    std::string address = "127.0.0.1";
    int port = 8080;
    bool debug = false;
    SegmentServer server(address, port, debug);
    auto f = [](int sig) {
        std::cout << "Server stopped. Exiting." << std::endl;
        running.store(false);
    };

    std::signal(SIGINT, f);
    std::signal(SIGTERM, f);
    // server.setReadHandler([](size_t,void const* data,size_t data_sz) {
    //     const unsigned char* bytes = (const unsigned char*)data;
    //     for (int i=0;i<data_sz;i++) {
    //         std::cout << bytes[i];
    //     }
    //     std::cout << std::endl;
    // });
    server.start();

    while (running)
        if (!server.isRunning()) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    server.stop();

    return 0;
}
