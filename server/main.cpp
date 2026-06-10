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
		auto serverConfig = handler.getJson(Configuration::Connection);
		std::string addr  = serverConfig.at("address").as_string().c_str();
		auto& portsArr    = serverConfig.at("port").as_array();
		auto& multiArr    = serverConfig.at("multiConnect").as_array();
		Logger::getInstance().log(LogLevel::Info, "ServerConfig",
		                          "address = " + addr + " port[0] = " + std::to_string(portsArr[0].as_int64()) +
		                              " multiConnect[0] = " + std::to_string(multiArr[0].as_bool()));

		params.port         = portsArr[0].as_int64();
		params.multiConnect = multiArr[0].as_bool();
	}
	catch(const std::exception& e) {
		std::cerr << e.what() << std::endl;
		params.port         = 12345;
		params.multiConnect = false;
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
	int id;

	while(running && server) {
		Logger::getInstance().log(LogLevel::Info, "Server::write", "Write ID : ");
		std::cin >> id;
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		Logger::getInstance().log(LogLevel::Info, "Server::write", "Write Message : ");
		std::getline(std::cin, msg);
		if(msg == "stop") {
			break;
		}

		server->write(id, msg.data(), msg.size());
		msg.clear();
	}
	server->stop();
	return 0;
}