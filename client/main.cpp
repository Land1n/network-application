//
// Created by ivan on 14.03.2026.
//
#include "SegmentClientCreator.hpp"
#include "ConfigurationHandler/ConfigurationHandler.hpp"
#include "clientserveriface/client.h"
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <cstring>

std::atomic<bool> running{true};

void signal_handler(int)
{
	running = false;
}

int main()
{
	std::signal(SIGINT, signal_handler);
	std::signal(SIGTERM, signal_handler);

	Network::ClientCreatorParams params;

	SegmentClientCreator creator;
	ConfigurationHandler handler;
	try {
		auto clientConfig = handler.getData(Configuration::Connection, User::Client);
		std::cout << "Client Config : " << clientConfig << std::endl;
		std::string addr = clientConfig.at("address").as_string().c_str();
		auto& portsArr   = clientConfig.at("ports").as_array();
		Logger::getInstance().log(LogLevel::Info, "ClientConfig",
		                          "address = " + addr + " port[0] = " + std::to_string(portsArr[0].as_int64()));
		params.hostname = addr;
		params.port     = portsArr[0].as_int64();
	}
	catch(const std::exception& e) {
		ErrorHandler::check_error(e, "ClientConfig");
		return -1;
	}

	auto net_client = creator.create(params);

	auto* client = reinterpret_cast<SegmentClient*>(net_client.get());
	client->setReadHandler([](const void* data, size_t sz) {
		std::string str(static_cast<const char*>(data), sz - 1);
		Logger::getInstance().log(LogLevel::Info, "Server::write", "RESPONSE : " + str);
	});

	client->start();

	std::string msg = "";
	while(running && client->getIsWork()) {
		Logger::getInstance().log(LogLevel::Info, "Client::write", " : ");
		std::getline(std::cin, msg);
		if(msg == "stop") {
			break;
		}
		client->write(msg.data(), msg.size());
	}

	client->stop();
	return 0;
}