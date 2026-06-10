//
// Created by guestuser on 05.06.2026.
//
#pragma once
#include "AcceptHandler/AcceptHandler.hpp"
#include "Session/Session.hpp"
#include "IOContextHandler/IOContextHandler.hpp"

#include <map>
#include <memory>

class SessionManager {
public:
	SessionManager(boost::asio::io_context& io_context, IOMode mode, const tcp::endpoint& endpoint);
	SessionManager(boost::asio::io_context& io_context, IOMode mode, const std::string& address, unsigned int port);
	SessionManager(boost::asio::io_context& io_context, IOMode mode, unsigned int port);
	~SessionManager();

	void addSession(unsigned int sessionID);

	size_t getSessionCount();

	std::shared_ptr<Session> getSession(unsigned int sessionID);

	void closeAcceptor();

	void setOnAccept(std::function<void(error_code)> onAccept);
	void setSessionOnReadHander(const std::function<void(std::size_t, const void*, size_t)>& function);

protected:
	void removeSession(unsigned int sessionID);

	IOMode mode;

	boost::asio::io_context& io_context;

	std::mutex sessionMapMutex;
	std::map<size_t, std::shared_ptr<Session>> sessionMap;

	std::function<void(std::size_t, const void*, size_t)> readHandlerForSession;

	std::unique_ptr<AcceptHandler> acceptHandler;
};