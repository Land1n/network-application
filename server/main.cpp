#include "SegmentServerCreator.hpp"
#include "clientserveriface/server.h"
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>

std::atomic<bool> running{true};

void signal_handler(int) {
    running = false;
}

int main() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    Network::ServerCreatorParams params;
    params.port = 8000;
    params.multiConnect = false;

    SegmentServerCreator creator;
    auto server = creator.create(params);
	Logger::getInstance().setLevel(LogLevel::Debug);
    server->setNewConnectionHandler([](Network::ConnectionId id) {
        std::cout << "[Server] New connection: " << id << std::endl;
    });
    server->setReadHandler([](Network::ConnectionId id, const void* data, size_t sz) {
        std::cout.write(static_cast<const char*>(data), sz) << std::endl;
    });
    server->start();

    while (running && server) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    server->stop();
    return 0;
}