//
// Created by guestuser on 28.05.2026.
//
#include "ErrorHandler/ErrorHandler.hpp"

#include <Logger.hpp>
#include <boost/json/array.hpp>

void log(LogLevel level, const std::string& function_name, const error_code& ec)
{
	Logger::getInstance().log(level, function_name, "Code = [ " + ec.message() + " ]");
}

bool ErrorHandler::check_error(const error_code& ec, const std::string& function_name, bool viewInfo)
{
	if(!ec) {
		if (viewInfo) {
			log(LogLevel::Info, function_name,ec);
		} else {
			log(LogLevel::Debug, function_name,ec);
		}
		return false;
	}
	if (ec == boost::asio::error::address_in_use) {
		log(LogLevel::Warn, function_name,ec);
		return false;
	}
	if (ec == boost::json::error::syntax) {
		log(LogLevel::Error, function_name,ec);
		return false;
	}
	if(ec == boost::asio::error::eof) {
		Logger::getInstance().log(LogLevel::Info, function_name, "Connection lost");
		return true;
	}
	if (ec == boost::asio::error::connection_refused) {
		log(LogLevel::Error, function_name,ec);
		return false;
	}
	return true;
}

bool ErrorHandler::check_error(const std::exception& ec, const std::string& function_name)
{

	Logger::getInstance().log(LogLevel::Critical, function_name, "Code = [ " + std::string(ec.what()) + " ]");
	return true;
}