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
//
// SegmentServer::SegmentServer(const std::string &address, int port, bool debug)
//     : address(address), port(port) {
//     connectionHandler = std::make_shared<ConnectionHandler>(
//         address, port, ConnectionHandlerType::Server, true
//     );
//     // messageHandler = std::make_shared<MessageHandler>(debug);
//     // logger.setLevel(debug ? LogLevel::Debug : LogLevel::Info);
//     connectionHandler->setTaskSocket([this](ConnectedSocket &cs) {
//         Logger::getInstance().setLevel(LogLevel::Trace);
//         TransportHandler transport(cs.ptr, 0xA0ABA0A, false);
//         if (readHandler)
//             transport.setOnReadHandler(readHandler);
//
//
//         while (connectionHandler->getIsWork()) {
//             auto tm = transport.read();
//             if (tm.type.empty()) break; // ошибка или разрыв
//
//             if (tm.type == "raw") {
//                 std::string w = "Hello client!";
//                 write(cs.id, w.data(), w.size());
//             }
//
//             auto msg = messageHandler->parse(tm);
//             // auto *raw_msg = dynamic_cast<RawMessage *>(msg.get());
//             // std::cout << __func__ <<"[Server] Sent: " << reinterpret_cast<char *>(raw_msg->getData().data()) << std::endl;
//
//             std::this_thread::sleep_for(std::chrono::seconds(3));
//         }
//         // std::cout << str_payload << std::endl;
//         // auto msg = messageHandler->parse(tm);
//         // auto *raw = dynamic_cast<RawMessage *>(msg.get());
//         // std::cout << reinterpret_cast<char*>(raw->getData().data()) << std::endl;
//         // std::string str_raw(raw->getData().begin(), raw->getData().end());
//         // auto sockPtr = sock.ptr;
//         // auto id = sock.id;
//         // /// TODO: убедиться, что не создаем больше потоков, чем есть сокетов в тесте
//         // /// (отследить по логам либо (рекомендуется) по счетчиу вызова callback
//         // //std::cerr << "thread created" <<std::endl;
//         // std::thread([this, sockPtr, id]() {
//         //     TransportHandler transport(sockPtr, 0xA0ABA0A, false);
//         //     /// +- TODO: condition
//         //     while (true) {
//         //         TransportMessage tm = transport.read();
//         //         if (tm.type.empty()) break; // соединение закрыто или ошибка
//         //         std::cerr << "processor called" <<std::endl;
//         //         // Парсим JSON из payload
//         //         std::string jsonStr(tm.payload.begin(), tm.payload.end());
//         //         try {
//         //             auto jv = boost::json::parse(jsonStr);
//         //             if (jv.is_object() && jv.as_object().contains("data")) {
//         //                 const auto& dataArr = jv.at("data").as_array();
//         //                 std::vector<uint8_t> buffer;
//         //                 buffer.reserve(dataArr.size());
//         //                 for (const auto& val : dataArr) {
//         //                     buffer.push_back(static_cast<uint8_t>(val.as_int64()));
//         //                 }
//         //                 if (readHandler) {
//         //                     // 2 вариант - std:atomic<uint32> counterCall
//         //                     // readHandler -> counterCall++;
//         //                     // TEST: ASSERT по кол-ву сокетов
//         //                     readHandler(id, buffer.data(), buffer.size());
//         //                 }
//         //             }
//         //         } catch (const std::exception& e) {
//         //             logger.log(LogLevel::Warn, "read_loop",
//         //                         "JSON parse error: " + std::string(e.what()));
//         //         }
//         //     }
//         // }).detach();
//     });
// }
//

//
// void SegmentServer::start() {
//     connectionHandler->start();
//     connectionHandler->listen();
// }
//
// void SegmentServer::stop() {
//     connectionHandler->stop();
// }
//
// void SegmentServer::write(Network::ConnectionId id, const void *data, size_t sz) {
//     auto sock = connectionHandler->findConnectedSocket(id);
//     if (!connectionHandler->getIsWork()) {
//         logger.log(LogLevel::Error, "write", "Cannot send data to the " + std::to_string(id));
//         return;
//     }
//
//     if (!sock.ptr || !sock.ptr->is_open()) {
//         logger.log(LogLevel::Warn, "write", "Socket " + std::to_string(id) + " not open");
//         return;
//     }
//
//     const uint8_t *bytes = static_cast<const uint8_t *>(data);
//     boost::json::array arr;
//     for (size_t i = 0; i < sz; ++i) {
//         arr.push_back(bytes[i]);
//     }
//
//     boost::json::object obj;
//     obj["type"] = "raw";
//     obj["transaction"] = static_cast<int>(Transaction::Request);
//     obj["data"] = std::move(arr);
//
//     std::string jsonStr = boost::json::serialize(obj);
//     std::vector<uint8_t> payload(jsonStr.begin(), jsonStr.end());
//     TransportMessage tm("raw", Transaction::Request, payload);
//     TransportHandler transport(sock.ptr);
//
//     if (!transport.write(tm)) {
//         logger.log(LogLevel::Error, "write", "Failed to send data to " + std::to_string(id));
//     }
//     std::cout << jsonStr << std::endl;
// }
//
//
//
// // void SegmentServer::setCloseConnectionHandler(ConnChangeHandler h) {
// //     closeHandler = std::move(h);
// //     connectionHandler->setCloseConnectionHandler([this](ConnectedSocket sock) {
// //         if (closeHandler) closeHandler(sock.id);
// //     });
// // }
// // /// + TODO: не переопределять лучше
// // void SegmentServer::setNewConnectionHandler(ConnChangeHandler h) {
// //     newHandler = std::move(h);
// //     connectionHandler->setNewConnectionHandler([this](ConnectedSocket sock) {
// //         if (newHandler) newHandler(sock.id);
// //     });
// // }
// //
// // void SegmentServer::setReadHandler(ReadHandler h) {
// //     readHandler = std::move(h);
// // }


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
        std::thread([this, socket]() {
            TransportHandler transport(socket.ptr);
            if (readHandler) transport.setOnReadHandler(readHandler);
            logger.setLevel(LogLevel::Debug);
            while (connection_handler->getIsWork()) {
                TransportMessage req = transport.read();
                if (req.transaction == Transaction::Error) break;

                auto msg = message_handler->parse(req);
                if (msg) {
                    auto resp = request_response_handler->processingRequestResponse(std::move(msg));
                    TransportMessage resp_msg = message_handler->serialize(std::move(resp));
                    if (!transport.write(resp_msg)) break;
                }
            }
            ConnectedSocket s = socket;
            connection_handler->disconnected(s, true);
        }).detach();
    });
    std::thread([this]() {
        if (!multiConnect)
            while (connection_handler->getIsWork()) {
                if (connection_handler && connection_handler->getSockets().size() > 1) {
                    connection_handler->disconnected(*(connection_handler->getSockets().end() - 1),true);
                }
            }
    }).detach();
    connection_handler->start();
    connection_handler->listen();
}

void SegmentServer::stop() {
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
