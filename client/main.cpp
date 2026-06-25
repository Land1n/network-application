//
// Created by ivan on 14.03.2026.
//
#include "segment_client/segment_client_creator.h"
#include "configuration_handler/configuration_handler.h"
#include "clientserveriface/client.h"
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <cstring>

#include "utils/alias.h"

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

	MagicInt magicNumber;
	try {
		auto clientConfig   = handler.getData(Configuration::Connection, User::Client);
		std::string address = clientConfig.at("address").as_string().c_str();
		auto& portsArr      = clientConfig.at("port").as_int64();
		magicNumber         = clientConfig.at("magic").to_number<MagicInt>();
		Logger::getInstance().log(LogLevel::Info, "ClientConfig", "ClientConfig = " + json::serialize(clientConfig));
		if(address == "") {
			throw std::runtime_error("Address not found");
		}

		params.hostname = address;
		params.port     = portsArr;
	}
	catch(const std::exception& e) {
		ErrorHandler::check_error(e, "ClientConfig");
		return -1;
	}
	Logger::getInstance().setLevel(LogLevel::Debug);
	auto net_client = creator.create(params);
	auto* client    = reinterpret_cast<SegmentClient*>(net_client.get());

	client->setMagicNumber(magicNumber);

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