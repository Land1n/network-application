#include "SegmentServer.hpp"
#include <iostream>
#include <thread>

int main() {
    // SegmentServer server("127.0.0.1", 8080); // debug enabled
    //
    // // Optional: enable automatic request-response processing
    // // server.enableAutoRequestResponse(true);
    //
    // server.setNewConnectionHandler([](Network::ConnectionId id) {
    //     std::cout << "[Server] New connection: " << id << std::endl;
    // });
    //
    // // server.setReadHandler([](Network::ConnectionId id, const void* data, size_t sz) {
    // //     // This will only be called for messages not automatically handled
    // //     std::string msg(static_cast<const char*>(data), sz);
    // //     std::cout << "[Server] Unhandled message from " << id << ": " << msg << std::endl;
    // // });
    //
    // server.setCloseConnectionHandler([](Network::ConnectionId id) {
    //     std::cout << "[Server] Connection closed: " << id << std::endl;
    // });
    //
    // server.start();
    // std::cout << "Server running. Press Enter to stop...\n";
    // std::cin.get();
    // server.stop();
    //
    // return 0;
}