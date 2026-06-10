//
// Created by ivan on 09.03.2026.
//
#include "SegmentServer.hpp"
#include "RawMessage.hpp"

#include <iostream>

SegmentServer::SegmentServer(const std::string& address, int port, bool multiConnect) :
    multiConnect(multiConnect), address(address), port(port)
{}
SegmentServer::SegmentServer(int port, bool multiConnect) : multiConnect(multiConnect), port(port)
{}

void SegmentServer::start()
{
	if(address != "") {
		sessionManager = std::make_unique<SessionManager>(ioContext.getIOContext(), mode, address, port);
	}
	else {
		sessionManager = std::make_unique<SessionManager>(ioContext.getIOContext(), mode, port);
	}
	isWork = true;

	if(readHandler)
		sessionManager->setSessionOnReadHander(readHandler);

	mainThread = std::thread([this]() {
		ErrorHandler::check_error(error_code{}, "SegmentServer::start", true);

		while(isWork) {
			size_t ID = IDDistributionHandler();
			sessionManager->addSession(ID);



			std::thread([this, ID]() {
				if(isWork) {
					auto session = sessionManager->getSession(ID);
					if(session != nullptr) {
						session->setOnAllRead([](error_code ec, std::unique_ptr<Message>&& msg) {
							if(msg != nullptr) {
								auto raw = static_cast<RawMessage*>(msg.get());
								std::string raw_msg(reinterpret_cast<const char*>(raw->getData().data()),
								                    raw->getData().size());
								Logger::getInstance().log(LogLevel::Info, "SegmentServer::read",
								                          "RESPONSE to string : " + raw_msg);
							}
						});
						while(isWork) {
							session->read();
						}
					}
				}



			}).detach();
		}
		ErrorHandler::check_error(error_code{}, "SegmentServer::stop", true);
	});
}

void SegmentServer::stop()
{
	if(!isWork)
		return;
	isWork.store(false);
	sessionManager->closeAcceptor();
	if(mainThread.joinable()) {
		mainThread.join();
	}
}

void SegmentServer::write(Network::ConnectionId id, const void* data, size_t sz)
{
	const uint8_t* bytes = static_cast<const uint8_t*>(data);
	std::vector<uint8_t> raw_bytes(bytes, bytes + sz);
	auto raw_msg = std::make_unique<RawMessage>("raw", Transaction::Response, raw_bytes);

	auto session = sessionManager->getSession(id);
	if(session != nullptr)
		session->write(std::move(raw_msg));
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
SegmentServer::~SegmentServer()
{
	stop();
}
//
// void SegmentServer::start()
// {
// 	connection_handler->setTaskSocket([this](ConnectedSocket& socket) {
// 		if(!multiConnect && connection_handler->getSockets().size() > 1) {
// 			logger.log(LogLevel::Info, "taskSocket", "Rejected extra connection, closing socket");
// 			disconnect(socket.id);
// 			return;
// 		}
//
// 		std::thread([this, socket]() {
// 			aliveThreads.fetch_add(1);
// 			TransportHandler transport(socket.ptr);
// 			if(readHandler)
// 				transport.setOnReadHandler(readHandler);
// 			while(connection_handler->getIsWork() && socket.ptr->is_open()) {
// 				TransportMessage req = transport.read();
// 				if(req.transaction == Transaction::Error)
// 					break;
//
// 				auto msg = message_handler->parse(req);
// 				if(msg) {
// 					auto resp                 = request_response_handler->processingRequestResponse(std::move(msg));
// 					TransportMessage resp_msg = message_handler->serialize(std::move(resp));
// 					if(!transport.write(resp_msg))
// 						break;
// 				}
// 			}
// 			aliveThreads.fetch_add(-1);
// 			ConnectedSocket s = socket;
//
// 			connection_handler->disconnected(s, true);
// 		}).detach();
// 	});
// 	connection_handler->start();
// 	connection_handler->listen();
// 	aliveThreads.fetch_add(2);
// }
//
// void SegmentServer::stop()
// {
// 	aliveThreads.fetch_add(-2);
// 	connection_handler->stop();
// }
//
// bool SegmentServer::isRunning()
// {
// 	return connection_handler->getIsWork();
// }
//
// void SegmentServer::write(Network::ConnectionId id, const void* data, size_t sz)
// {
// 	if(!connection_handler->getIsWork()) {
// 		logger.log(LogLevel::Critical, __func__, "Write fail");
// 		return;
// 	}
//
// 	const uint8_t* bytes = static_cast<const uint8_t*>(data);
// 	std::vector<uint8_t> raw_bytes(bytes, bytes + sz);
// 	auto raw_msg        = std::make_unique<RawMessage>("raw", Transaction::Response, raw_bytes);
// 	TransportMessage tm = message_handler->serialize(std::move(raw_msg));
// 	if(tm.payload.empty()) {
// 		logger.log(LogLevel::Error, __func__, "Failed to serialize raw message");
// 		return;
// 	}
//
// 	auto sock = connection_handler->findConnectedSocket(id);
// 	if(!sock.ptr) {
// 		logger.log(LogLevel::Error, __func__, "Socket not found");
// 		return;
// 	}
// 	TransportHandler transport_handler(sock.ptr);
// 	transport_handler.setOnReadHandler(readHandler);
// 	transport_handler.write(tm);
// }
//
// void SegmentServer::disconnect(Network::ConnectionId id)
// {
// 	try {
// 		auto sock = connection_handler->findConnectedSocket(id);
// 		if(sock.ptr) {
// 			connection_handler->disconnected(sock, true);
// 			if(closeHandler)
// 				closeHandler(id);
// 		}
// 	}
// 	catch(const std::exception& e) {
// 		logger.log(LogLevel::Error, __func__, "Socket not found");
// 	}
// }
//
// void SegmentServer::setIdDistributionHandler(IdDistributionHandler h)
// {
// 	connection_handler->setGenerateID(h);
// }
//
// int SegmentServer::getAliveThreads()
// {
// 	return aliveThreads.load();
// }
//
// std::vector<ConnectedSocket> SegmentServer::getConnectedClients()
// {
// 	return connection_handler->getSockets();
// }
