//
// Created by guestuser on 22.06.2026.
//
//
// Created by guestuser on 28.05.2026.
//
#include "utils/error_handler.h"

#include "logger/logger.h"

#include <boost/json.hpp>
#include <boost/asio.hpp>

void log(LogLevel level, const std::string& function_name, const error_code& ec)
{
	Logger::getInstance().log(level, function_name, "Code = [ " + ec.message() + " ]");
}

bool ErrorHandler::check_error(const error_code& ec, const std::string& function_name, bool viewInfo)
{
	if(!ec) {
		if(viewInfo) {
			log(LogLevel::Info, function_name, ec);
		}
		else {
			log(LogLevel::Debug, function_name, ec);
		}
		return false;
	}
	if(ec == boost::asio::error::address_in_use) {
		if(viewInfo)
			log(LogLevel::Warn, function_name, ec);
		else
			log(LogLevel::Debug, function_name, ec);

		return false;
	}
	if(ec == boost::json::error::syntax) {
		if(viewInfo)
			log(LogLevel::Error, function_name, ec);
		else
			log(LogLevel::Debug, function_name, ec);
		return false;
	}
	if(ec == boost::asio::error::eof) {
		if(viewInfo) {
			Logger::getInstance().log(LogLevel::Info, function_name, "Code = [ Connection lost ]");
		}
		else {
			Logger::getInstance().log(LogLevel::Debug, function_name, "Code = [ Connection lost ]");
		}
		return true;
	}
	if(ec == boost::asio::error::bad_descriptor) {
		if(viewInfo)
			log(LogLevel::Error, function_name, ec);
		else
			log(LogLevel::Debug, function_name, ec);
		return true;
	}
	if(ec == boost::asio::error::connection_refused) {
		if(viewInfo)
			log(LogLevel::Error, function_name, ec);
		else
			log(LogLevel::Debug, function_name, ec);
		return false;
	}
	return true;
}

bool ErrorHandler::check_error(const std::exception& ec, const std::string& function_name)
{
	Logger::getInstance().log(LogLevel::Critical, function_name, "Code = [ " + std::string(ec.what()) + " ]");
	return true;
}