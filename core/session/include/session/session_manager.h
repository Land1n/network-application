//
// Created by guestuser on 05.06.2026.
//
#pragma once
#include "connection/accept_handler.h"
#include "session/session.h"
#include "io_context_handler/io_context_handler.h"
#include "utils/alias.h"
#include "clientserveriface/server.h"

#include <map>
#include <memory>

class SessionManager {
public:
	using SessionIDInt = uint32_t;
	using SessionMap   = std::map<SessionIDInt, std::shared_ptr<Session>>;

	SessionManager(boost::asio::io_context& io_context, IOMode mode, const tcp::endpoint& endpoint);
	SessionManager(boost::asio::io_context& io_context, IOMode mode, const std::string& address, PortInt port);
	SessionManager(boost::asio::io_context& io_context, IOMode mode, PortInt port);
	~SessionManager();

	void addSession(SessionIDInt sessionID);

	size_t getSessionCount();

	std::shared_ptr<Session> getSession(SessionIDInt sessionID);

	void closeAcceptor();

	void setOnAccept(const CallbackError& onAccept);
	void setSessionOnReadHandler(const Network::Server::ReadHandler& function);

	void setMagicNumber(MagicInt magicNumber);

	SessionMap getSessionMap();

protected:
	void removeSession(unsigned int sessionID);

	IOMode mode;

	boost::asio::io_context& io_context;

	std::mutex sessionMapMutex;
	SessionMap sessionMap;

	Network::Server::ReadHandler readHandlerForSession;

	MagicInt magicNumber;

	std::unique_ptr<AcceptHandler> acceptHandler;
};