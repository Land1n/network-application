#include "SegmentServerCreator.hpp"
#include "../../clientserveriface/include/clientserveriface/server.h"
#include "include/SegmentServerCreator.hpp"

#include "ConfigurationHandler/ConfigurationHandler.hpp"

#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>

std::atomic<bool> running{true};

void signal_handler(int)
{
	running = false;
}

int main()
{
	std::signal(SIGINT, signal_handler);
	std::signal(SIGTERM, signal_handler);

	Network::ServerCreatorParams params;

	ConfigurationHandler handler;
	try {
		auto serverConfig = handler.getData(Configuration::Connection, User::Server);
		auto& portsArr    = serverConfig.at("ports").as_array();
		auto& multiArr    = serverConfig.at("multiConnect").as_array();
		Logger::getInstance().log(LogLevel::Info, "ServerConfig",
		                          "port[0] = " + std::to_string(portsArr[0].as_int64()) +
		                              " multiConnect[0] = " + std::to_string(multiArr[0].as_bool()));

		params.port         = portsArr[0].as_int64();
		params.multiConnect = multiArr[0].as_bool();
	}
	catch(const std::exception& e) {
		Logger::getInstance().log(LogLevel::Warn, "ServerConfig ", "default");
	}

	SegmentServerCreator creator;
	auto server = creator.create(params);

	std::atomic<int> index{0};

	auto handlerID = [&index]() {
		return ++index;
	};
	server->setIdDistributionHandler(handlerID);

	server->setReadHandler([](Network::ConnectionId id, const void* data, size_t sz) {
		std::string str(static_cast<const char*>(data), sz - 1);
		Logger::getInstance().log(LogLevel::Info, "Client::write", "RESPONSE : " + str);
	});
	server->start();

	std::string msg;

	while(running && server) {
		Logger::getInstance().log(LogLevel::Info, "Server::write", "Write ID;Message  : ");
		std::getline(std::cin, msg);
		int id = 0;
		std::string message;

		size_t pos = msg.find(';');
		if(pos != std::string::npos) {
			std::string id_part = msg.substr(0, pos);
			message             = msg.substr(pos + 1);

			try {
				id = std::stoi(id_part);
				if(message == "stop") {
					break;
				}

				server->write(id, message.data(), message.size());
				msg.clear();
			}
			catch(const std::exception& e) {
				Logger::getInstance().log(LogLevel::Error, "Server::write", "Invalid ID: " + id_part);
			}
		}
		else {
			Logger::getInstance().log(LogLevel::Error, "Server::write", "Missing ';' delimiter");
		}
	}
	server->stop();
	return 0;
}