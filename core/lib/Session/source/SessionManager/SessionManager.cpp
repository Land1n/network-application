//
// Created by guestuser on 05.06.2026.
//

#include "SessionManager/SessionManager.hpp"
#include "Logger.hpp"
#include <iostream>

SessionManager::SessionManager(boost::asio::io_context& io_context, IOMode mode, const tcp::endpoint& endpoint) :
    io_context(io_context), mode(mode)
{
	acceptHandler = std::make_unique<AcceptHandler>(io_context, endpoint);
}
SessionManager::SessionManager(boost::asio::io_context& io_context, IOMode mode, const std::string& address,
                               unsigned int port) :
    io_context(io_context), mode(mode)
{}
SessionManager::SessionManager(boost::asio::io_context& io_context, IOMode mode, unsigned int port) :
    io_context(io_context), mode(mode)
{
	acceptHandler = std::make_unique<AcceptHandler>(io_context, port);
}
SessionManager::~SessionManager()
{
	acceptHandler->setOnAccept(nullptr);
	acceptHandler->setOnError(nullptr);

	ErrorHandler::check_error(error_code(),
	                          "SessionManager::SessionMap{Count=" + std::to_string(getSessionCount()) + "}");
	auto copy_sessionMap = sessionMap;

	for(auto& [id, session]: copy_sessionMap) {
		if(session && session != nullptr) {
			session->setOnError(nullptr);
			session->disconnect();
		}
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

	if(readHandlerForSession) {
		session->setOnReadHandler(readHandlerForSession);
	}

	session->setOnError([this, sessionID](error_code ec) {
		removeSession(sessionID);
	});

	session->setOnAccept([this, sessionID, session](error_code ec) {
		if(ec) {
			if(sessionMap.find(sessionID) != sessionMap.end()) {
				removeSession(sessionID);
			}
		}
		else {
			std::unique_lock lk(sessionMapMutex);
			sessionMap[sessionID] = session;
			ErrorHandler::check_error(error_code(),
			                          "SessionManager::addSession{sessionID=" + std::to_string(sessionID) + "}", true);
		}
	});

	session->accept(*acceptHandler);
}

void SessionManager::removeSession(unsigned int sessionID)
{
	if(getSession(sessionID) == nullptr)
		return;
	std::unique_lock<std::mutex> lk(sessionMapMutex);
	ErrorHandler::check_error(error_code(),
	                          "SessionManager::removeSession{sessionID=" + std::to_string(sessionID) + "}", true);
	sessionMap.erase(sessionID);
}

std::shared_ptr<Session> SessionManager::getSession(unsigned int sessionID)
{
	std::unique_lock<std::mutex> lk(sessionMapMutex);
	if(sessionMap.find(sessionID) == sessionMap.end()) {
		Logger::getInstance().log(LogLevel::Warn,
		                          "SessionManager::getSession{sessionID=" + std::to_string(sessionID) + "}",
		                          "Code = [ Not Found ]");

		return nullptr;
	}

	return sessionMap[sessionID];
}
void SessionManager::closeAcceptor()
{
	acceptHandler->close();
}
void SessionManager::setOnAccept(std::function<void(error_code)> handler)
{
	acceptHandler->setOnAccept(handler);
}
void SessionManager::setSessionOnReadHander(const std::function<void(std::size_t, const void*, size_t)>& function)
{
	readHandlerForSession = function;
}

size_t SessionManager::getSessionCount()
{
	std::unique_lock<std::mutex> lk(sessionMapMutex);
	return sessionMap.size();
}