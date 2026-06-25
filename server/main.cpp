#include "segment_server/segment_server_creator.h"
#include "clientserveriface/server.h"

#include "configuration_handler/configuration_handler.h"

#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>

std::atomic<bool> running{true};

void signal_handler(int)
{
	running = false;
}

std::map<uint32_t, std::shared_ptr<Network::Server>> servers;

int main()
{
	std::signal(SIGINT, signal_handler);
	std::signal(SIGTERM, signal_handler);

	Network::ServerCreatorParams params;

	std::atomic<int> index{0};

	auto handlerID = [&index]() {
		return ++index;
	};

	auto alIslWork = []() {
		for(auto server: servers) {
			auto segment_server = dynamic_cast<SegmentServer*>(server.second.get());
			if(!segment_server->getIsWork())
				return false;
		}
		return true;
	};

	auto allStop = []() {
		for(auto server: servers) {
			server.second->stop();
		}
	};
	auto allStart = []() {
		for(auto server: servers) {
			server.second->start();
		}
	};

	ConfigurationHandler handler;

	Logger::getInstance().setLevel(LogLevel::Debug);

	try {
		auto serverConfig = handler.getData(Configuration::Connection, User::Server);
		auto& portsArr    = serverConfig.at("ports").as_array();
		auto& multiArr    = serverConfig.at("multiConnect").as_array();
		auto& magicArr    = serverConfig.at("magic").as_array();
		Logger::getInstance().log(LogLevel::Info, "ServerConfig", "Config = [ " + json::serialize(serverConfig) + " ]");

		for(short i = 0; i < portsArr.size(); i++) {
			params.port          = portsArr.at(i).as_int64();
			params.multiConnect  = multiArr.at(i).as_bool();
			uint32_t magicNumber = magicArr.at(i).to_number<uint32_t>();
			SegmentServerCreator creator;
			auto server = creator.create(params);

			auto* segmentServer = dynamic_cast<SegmentServer*>(server.get());

			segmentServer->setMagicNumber(magicNumber);

			segmentServer->setIdDistributionHandler(handlerID);

			segmentServer->setReadHandler([](Network::ConnectionId id, const void* data, size_t sz) {
				std::string str(static_cast<const char*>(data), sz - 1);
				Logger::getInstance().log(LogLevel::Info, "Client::write", "RESPONSE : " + str);
			});

			servers[params.port] = server;
		}
	}
	catch(const std::exception& e) {
		ErrorHandler::check_error(e, "serverConfig");
		return -1;
	}

	allStart();

	std::string msg;

	while(running && alIslWork()) {
		while(running && alIslWork()) {
			Logger::getInstance().log(LogLevel::Info, "Server::write", "Write SERVER_PORT;ID_SESSION;MESSAGE : ");
			std::string line;
			std::getline(std::cin, line);

			if(line == "stop") {
				break;
			}

			size_t pos1 = line.find(';');
			if(pos1 == std::string::npos) {
				Logger::getInstance().log(LogLevel::Error, "Server::write", "Missing first ';' delimiter");
				continue;
			}
			size_t pos2 = line.find(';', pos1 + 1);
			if(pos2 == std::string::npos) {
				Logger::getInstance().log(LogLevel::Error, "Server::write", "Missing second ';' delimiter");
				continue;
			}

			std::string id_server_part  = line.substr(0, pos1);
			std::string id_session_part = line.substr(pos1 + 1, pos2 - pos1 - 1);
			std::string message         = line.substr(pos2 + 1);

			int id_server  = 0;
			int id_session = 0;
			try {
				id_server  = std::stoi(id_server_part);
				id_session = std::stoi(id_session_part);
			}
			catch(const std::exception& e) {
				Logger::getInstance().log(LogLevel::Error, "Server::write",
				                          "Invalid integer conversion: " + std::string(e.what()));
				continue;
			}
			if(servers.find(id_server) != servers.end()) {
				servers[id_server]->write(id_session, message.data(), message.size());
			}
			else {
				Logger::getInstance().log(LogLevel::Error, "Server::write", "Invalid port");
			}
		}

		allStop();

		return 0;
	}
}