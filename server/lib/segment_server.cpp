//
// Created by ivan on 09.03.2026.
//
#include "segment_server/segment_server.h"
#include "message/raw_message.h"
#include "request_response_handler/request_response_handler.h"

#include <iostream>

namespace {

std::atomic<int> ID{0};

auto handlerID = []() {
	ID.fetch_add(1);
	return ID.load();
};

} // namespace

SegmentServer::SegmentServer(const std::string& address, PortInt port, bool multiConnect) :
    multiConnect(multiConnect), address(address), port(port)
{}
SegmentServer::SegmentServer(int port, bool multiConnect) : multiConnect(multiConnect), port(port)
{}

void SegmentServer::start()
{
	if(!address.empty()) {
		sessionManager = std::make_unique<SessionManager>(ioContext.getIOContext(), mode, address, port);
	}
	else {
		sessionManager = std::make_unique<SessionManager>(ioContext.getIOContext(), mode, port);
	}

	if(!IDDistributionHandler) {
		setIdDistributionHandler(handlerID);
	}

	if(readHandler)
		sessionManager->setSessionOnReadHandler(readHandler);
	if(magicNumber)
		sessionManager->setMagicNumber(magicNumber);

	mainWorker.addTask([this]() {
		while(isWork) {
			size_t ID = IDDistributionHandler();
			sessionManager->addSession(ID);
			if(!multiConnect && sessionManager->getSessionCount() > 1) {
				disconnect(ID);
			}
			else {
				if(isWork) {
					std::unique_ptr<Worker> sessionWorker = std::make_unique<Worker>(10000);
					sessionWorker->addTask([this, ID]() {
						sessionWork(ID);
					});
					sessionWorkers.push_back(std::move(sessionWorker));
				}
			}
		}
	});
	isWork = true;

	ErrorHandler::check_error(error_code{}, "SegmentServer::start", true);
}

void SegmentServer::stop()
{
	if(!isWork)
		return;
	isWork.store(false);
	sessionManager->closeAcceptor();

	auto copy_map = sessionManager->getSessionMap();
	if(sessionManager->getSessionCount()) {
		for(auto [id, session]: copy_map) {
			session->disconnect();
		}
	}

	mainWorker.flush();
	for(auto& worker: sessionWorkers) {
		worker->flush();
	}
	ErrorHandler::check_error(error_code{}, "SegmentServer::stop", true);
}

void SegmentServer::write(Network::ConnectionId id, const void* data, size_t sz)
{
	auto session = sessionManager->getSession(id);

	std::string type(static_cast<const char*>(data), sz);
	if(type == "signal" || type == "information") {
		if(session != nullptr)
			session->write(std::make_unique<Message>(type, Transaction::Request));
	}
	else {
		const auto* bytes = static_cast<const uint8_t*>(data);
		std::vector<uint8_t> raw_bytes(bytes, bytes + sz);
		auto raw_msg = std::make_unique<RawMessage>("raw", Transaction::Response, raw_bytes);
		if(session != nullptr)
			session->write(std::move(raw_msg));
	}
}
void SegmentServer::disconnect(Network::ConnectionId id)
{
	auto session = sessionManager->getSession(id);
	session->disconnect();
}
void SegmentServer::setIdDistributionHandler(IdDistributionHandler h)
{
	IDDistributionHandler = h;
}
void SegmentServer::setIOMode(IOMode ioMode)
{
	mode = ioMode;
}
void SegmentServer::setMagicNumber(uint32_t n)
{
	magicNumber = n;
}
bool SegmentServer::getIsWork()
{
	return isWork.load();
}
std::vector<size_t> SegmentServer::getSessionVectorID()
{
	std::vector<size_t> result;
	for(auto& [id, session]: sessionManager->getSessionMap()) {
		result.emplace_back(id);
	}
	return result;
}

void SegmentServer::sessionWork(size_t sessionId)
{
	auto session = sessionManager->getSession(sessionId);

	if(session != nullptr) {
		while(isWork && !session->hasError()) {
			session->setOnAllRead([&session, sessionId](error_code ec, std::unique_ptr<Message>&& msg) {
				if(msg != nullptr) {
					if(msg->type == "raw") {
						auto raw = dynamic_cast<RawMessage*>(msg.get());
						std::string raw_msg(reinterpret_cast<const char*>(raw->getData().data()),
						                    raw->getData().size());
						Logger::getInstance().log(LogLevel::Info, "SegmentServer::read",
						                          "RESPONSE to string [SessionID=" + std::to_string(sessionId) +
						                              "] : " + raw_msg);
					}
					else {
						auto new_msg = RequestResponseHandler::createResponse(std::move(msg));
						session->write(new_msg);
					}
				}
			});
			session->read();
		}
	}
}
SegmentServer::~SegmentServer()
{
	stop();
}