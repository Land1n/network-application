//
// Created by guestuser on 21.06.2026.
//
#include "request_response_handler/request_response_handler.h"

#include "message/information_message.h"
#include "logger/logger.h"
#include "message/signal_message.h"
#include <complex>
#include <random>

namespace RequestResponseHandler {
std::unique_ptr<Message> createRequest(std::string&& type_message)
{
	Logger::getInstance().log(LogLevel::Debug, "RequestResponseHandler::createRequest",
	                          "Message.type = [ " + type_message + " ]");
	return std::make_unique<Message>(type_message, Transaction::Request);
}
std::unique_ptr<Message> createResponse(std::unique_ptr<Message>&& message)
{
	if(message == nullptr) {
		Logger::getInstance().log(LogLevel::Error, "RequestResponseHandler::createResponse", "Message = [ nullptr ]");
		return std::make_unique<Message>("unknown", Transaction::Error);
	}
	if(message->transaction == Transaction::Response) {
		Logger::getInstance().log(LogLevel::Warn, "RequestResponseHandler::createResponse",
		                          "Message.transaction = [ Response ]");
		return std::make_unique<Message>("unknown", Transaction::Error);
	}
	Logger::getInstance().log(LogLevel::Debug, "RequestResponseHandler::createResponse",
	                          "Message.transaction = [ Request ]");
	Logger::getInstance().log(LogLevel::Debug, "RequestResponseHandler::createResponse",
	                          "Message.type = [ " + message->type + " ]");

	// тут мы как-то по умному получаем данные
	if(message->type == "signal") {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> dist_n(5, 20);
		std::uniform_real_distribution<float> dist_f(0.0f, 10.0f);
		std::uniform_int_distribution<int> dist(3000, 10000);
		int N_number = dist_n(gen);

		std::vector<std::complex<float>> fake_data;
		for(int i = 0; i < N_number; i++) {
			fake_data.emplace_back(dist_f(gen), dist_f(gen));
		}
		Logger::getInstance().log(LogLevel::Debug, "RequestResponseHandler::createResponse", "Code = [ Success ]");
		return std::make_unique<SignalMessage>("signal", Transaction::Response, dist(gen), fake_data);
	}
	if(message->type == "information") {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> dist_n(5, 20);
		return std::make_unique<InformationMessage>("information", Transaction::Response, dist_n(gen));
	}
	Logger::getInstance().log(LogLevel::Warn, "RequestResponseHandler::createResponse", "Code = [ Success ]");
	return std::make_unique<Message>("unknown", Transaction::Error);
}
} // namespace RequestResponseHandler
