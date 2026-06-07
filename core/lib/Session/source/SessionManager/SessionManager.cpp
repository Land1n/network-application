//
// Created by guestuser on 05.06.2026.
//

#include "SessionManager/SessionManager.hpp"
#include "Logger.hpp"
#include <iostream>
#include <bits/codecvt.h>

SessionManager::SessionManager(boost::asio::io_context& io_context, IOMode mode, const tcp::endpoint& endpoint) :
    io_context(io_context), mode(mode)
{
	acceptHandler = std::make_unique<AcceptHandler>(io_context, endpoint);
}
SessionManager::SessionManager(boost::asio::io_context& io_context, IOMode mode, const std::string& address,
                               unsigned int port) :
    io_context(io_context), mode(mode)
{
	acceptHandler = std::make_unique<AcceptHandler>(io_context, address, port);
}
SessionManager::~SessionManager()
{
	acceptHandler->setOnAccept(nullptr);
	acceptHandler->setOnError(nullptr);

	ErrorHandler::check_error(error_code(),
	                          "SessionManager::SessionMap{Count=" + std::to_string(getSessionCount()) + "}");
	auto copy_sessionMap = sessionMap;

	for(auto& [id, session]: copy_sessionMap) {
		session->setOnError(nullptr);
		session->disconnect();
		removeSession(id);
	}
	acceptHandler->close();
	ErrorHandler::check_error(error_code(),
	                          "SessionManager::SessionMap{Count=" + std::to_string(getSessionCount()) + "}");

	ErrorHandler::check_error(error_code(), "SessionManager::~SessionManager");
}
void SessionManager::addSession(unsigned int sessionID)
{
	if(sessionMap.find(sessionID) != sessionMap.end()) {
		Logger::getInstance().log(LogLevel::Warn, "SessionManager::addSession", "Code = [ Already Exists ]");
		return;
	}
	auto session = std::make_shared<Session>(io_context, mode);

	session->setOnError([this, sessionID](error_code ec) {
		removeSession(sessionID);
	});

	session->setOnAccept([this, sessionID](error_code ec) {
		if(ec) {
			removeSession(sessionID);
		}
	});

	session->accept(*acceptHandler);
	{
		std::unique_lock lk(sessionMapMutex);
		sessionMap[sessionID] = session;
		ErrorHandler::check_error(error_code(),
		                          "SessionManager::addSession{sessionID=" + std::to_string(sessionID) + "}", true);
	}
}

void SessionManager::removeSession(unsigned int sessionID)
{
	std::unique_lock<std::mutex> lk(sessionMapMutex);
	ErrorHandler::check_error(error_code(),
	                          "SessionManager::removeSession{sessionID=" + std::to_string(sessionID) + "}", true);
	sessionMap.erase(sessionID);
}

std::shared_ptr<Session> SessionManager::getSession(unsigned int sessionID)
{
	if(sessionMap.find(sessionID) == sessionMap.end()) {
		Logger::getInstance().log(LogLevel::Warn,
		                          "SessionManager::addSession{sessionID=" + std::to_string(sessionID) + "}",
		                          "Code = [ Not Found ]");
		return nullptr;
	}

	std::unique_lock<std::mutex> lk(sessionMapMutex);
	return sessionMap[sessionID];
}

size_t SessionManager::getSessionCount()
{
	std::unique_lock<std::mutex> lk(sessionMapMutex);
	return sessionMap.size();
}