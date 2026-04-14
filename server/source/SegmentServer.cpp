//
// Created by ivan on 09.03.2026.
//

#include "SegmentServer.hpp"
#include "TransportHandler.hpp"
// server/source/SegmentServer.cpp
#include "SegmentServer.hpp"
#include "TransportHandler.hpp"
#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <thread>

SegmentServer::SegmentServer(const std::string& address, int port, bool debug)
    : address(address), port(port)
{
    connectionHandler = std::make_shared<ConnectionHandler>(
        address, port, ConnectionHandlerType::Server, !debug
    );
    logger = LoggerFactory::getLogger("SegmentServer");
    logger->setLevel(debug ? LogLevel::Debug : LogLevel::Info);

    // Задаём обработчик для каждого нового подключения
    connectionHandler->setTaskSocket([this](ConnectedSocket& sock) {
        auto sockPtr = sock.ptr;
        auto id = sock.id;

        std::thread([this, sockPtr, id]() {
            TransportHandler transport(sockPtr, 0xA0ABA0A, false);
            while (true) {
                TransportMessage tm = transport.read();
                if (tm.type.empty()) break; // соединение закрыто или ошибка

                // Парсим JSON из payload
                std::string jsonStr(tm.payload.begin(), tm.payload.end());
                try {
                    auto jv = boost::json::parse(jsonStr);
                    if (jv.is_object() && jv.as_object().contains("data")) {
                        const auto& dataArr = jv.at("data").as_array();
                        std::vector<uint8_t> buffer;
                        buffer.reserve(dataArr.size());
                        for (const auto& val : dataArr) {
                            buffer.push_back(static_cast<uint8_t>(val.as_int64()));
                        }
                        if (readHandler) {
                            readHandler(id, buffer.data(), buffer.size());
                        }
                    }
                } catch (std::exception& e) {
                    logger->log(LogLevel::Warn, "read_loop",
                                "JSON parse error: " + std::string(e.what()));
                }
            }
            // Чистим mutex
            {
                std::lock_guard<std::mutex> lock(socketMutexesMutex);
                socketMutexes.erase(id);
            }
        }).detach();
    });
}

SegmentServer::~SegmentServer() {
    stop();
}

void SegmentServer::start() {
    connectionHandler->start();
    connectionHandler->listen();
}

void SegmentServer::stop() {
    connectionHandler->stop();
}

void SegmentServer::write(Network::ConnectionId id, const void *data, size_t sz) {
    auto sock = connectionHandler->findConnectedSocket(id);
    if (!sock.ptr || !sock.ptr->is_open()) {
        logger->log(LogLevel::Warn, "write", "Socket " + std::to_string(id) + " not open");
        return;
    }

    // Формируем JSON с массивом чисел
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    boost::json::array arr;
    for (size_t i = 0; i < sz; ++i) {
        arr.push_back(bytes[i]);
    }

    boost::json::object obj;
    obj["type"] = "raw";
    obj["transaction"] = static_cast<int>(Transaction::Request);
    obj["data"] = std::move(arr);

    std::string jsonStr = boost::json::serialize(obj);
    std::vector<uint8_t> payload(jsonStr.begin(), jsonStr.end());
    TransportMessage tm("raw", Transaction::Request, payload);

    // Потокобезопасная запись
    std::mutex* mtx = nullptr;
    {
        std::lock_guard<std::mutex> lock(socketMutexesMutex);
        mtx = &socketMutexes[id];
    }
    std::lock_guard<std::mutex> lock(*mtx);

    TransportHandler transport(sock.ptr);
    if (!transport.write(tm)) {
        logger->log(LogLevel::Error, "write", "Failed to send data to " + std::to_string(id));
    }
}

void SegmentServer::disconnect(Network::ConnectionId id) {
    auto sock = connectionHandler->findConnectedSocket(id);
    if (sock.ptr) {
        connectionHandler->disconnected(sock, true);
    }
}

void SegmentServer::setIdDistributionHandler(IdDistributionHandler h) {
    connectionHandler->setGenerateID(h);
}

void SegmentServer::setCloseConnectionHandler(ConnChangeHandler h) {
    closeHandler = std::move(h);
    connectionHandler->setCloseConnectionHandler([this](ConnectedSocket sock) {
        {
            std::lock_guard<std::mutex> lock(socketMutexesMutex);
            socketMutexes.erase(sock.id);
        }
        if (closeHandler) closeHandler(sock.id);
    });
}

void SegmentServer::setNewConnectionHandler(ConnChangeHandler h) {
    newHandler = std::move(h);
    connectionHandler->setNewConnectionHandler([this](ConnectedSocket sock) {
        if (newHandler) newHandler(sock.id);
    });
}

void SegmentServer::setReadHandler(ReadHandler h) {
    readHandler = std::move(h);
}