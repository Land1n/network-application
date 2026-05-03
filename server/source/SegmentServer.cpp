//
// Created by ivan on 09.03.2026.
//

#include "SegmentServer.hpp"
#include "TransportHandler.hpp"
// server/source/SegmentServer.cpp
#include "SegmentServer.hpp"

#include <queue>

#include "TransportHandler.hpp"
#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <thread>

#include <iostream>

#include "RawMessage.hpp"
#include "ServerRequestResponseHandler.hpp"


SegmentServer::SegmentServer(const std::string &address, int port, bool multiConnect, bool debug) : address(address),
    port(port), multiConnect(multiConnect) {
    connection_handler = std::make_unique<ConnectionHandler>(
        address, port, ConnectionHandlerType::Server, true
    );
    message_handler = std::make_unique<MessageHandler>();
    request_response_handler = std::make_unique<ServerRequestResponseHandler>(message_handler->creator_message);
    logger.setLevel(LogLevel::Debug);
}

SegmentServer::~SegmentServer() {
    stop();
}

void SegmentServer::start() {
    connection_handler->setTaskSocket([this](ConnectedSocket &socket) {

        if (!multiConnect && connection_handler->getSockets().size() > 1) {
            logger.log(LogLevel::Info, "taskSocket", "Rejected extra connection, closing socket");
            disconnect(socket.id);
            return;
        }

        std::thread([this, socket]() {
            aliveThreads.fetch_add(1);
            TransportHandler transport(socket.ptr);
            if (readHandler) transport.setOnReadHandler(readHandler);
            logger.setLevel(LogLevel::Debug);
            while (connection_handler->getIsWork() && socket.ptr->is_open()) {
                TransportMessage req = transport.read();
                if (req.transaction == Transaction::Error) break;

                auto msg = message_handler->parse(req);
                if (msg) {
                    auto resp = request_response_handler->processingRequestResponse(std::move(msg));
                    TransportMessage resp_msg = message_handler->serialize(std::move(resp));
                    if (!transport.write(resp_msg)) break;
                }
            }
            aliveThreads.fetch_add(-1);
            ConnectedSocket s = socket;

            connection_handler->disconnected(s, true);
        }).detach();
    });
    connection_handler->start();
    connection_handler->listen();
    aliveThreads.fetch_add(2);

}

void SegmentServer::stop() {
    aliveThreads.fetch_add(-2);
    connection_handler->stop();
}

bool SegmentServer::isRunning() {
    return connection_handler->getIsWork();
}

void SegmentServer::write(Network::ConnectionId id, const void *data, size_t sz) {
    if (!connection_handler->getIsWork()) {
        logger.log(LogLevel::Critical, __func__, "Write fail");
        return;
    }

    const uint8_t *bytes = static_cast<const uint8_t *>(data);
    std::vector<uint8_t> raw_bytes(bytes, bytes + sz);
    auto raw_msg = std::make_unique<RawMessage>("raw", Transaction::Response, raw_bytes);
    TransportMessage tm = message_handler->serialize(std::move(raw_msg));
    if (tm.payload.empty()) {
        logger.log(LogLevel::Error, __func__, "Failed to serialize raw message");
        return;
    }

    auto sock = connection_handler->findConnectedSocket(id);
    if (!sock.ptr) {
        logger.log(LogLevel::Error, __func__, "Socket not found");
        return;
    }
    TransportHandler transport_handler(sock.ptr);
    transport_handler.setOnReadHandler(readHandler);
    transport_handler.write(tm);
}

void SegmentServer::disconnect(Network::ConnectionId id) {
    auto sock = connection_handler->findConnectedSocket(id);
    if (sock.ptr) {
        connection_handler->disconnected(sock, true);
        if (closeHandler) closeHandler(id);
    }
}

void SegmentServer::setIdDistributionHandler(IdDistributionHandler h) {
    connection_handler->setGenerateID(h);
}

int SegmentServer::getAliveThreads() {
    return aliveThreads.load();
}

int SegmentServer::getConnectedClients() {
    return connection_handler->getSockets().size();
}
