#include "SegmentClient.hpp"
#include <boost/json.hpp>
#include <thread>
#include <chrono>

SegmentClient::SegmentClient(const std::string& serverAddress, int serverPort, bool debug)
    : serverAddress(serverAddress), serverPort(serverPort), debug(debug)
{
    connectionHandler = std::make_shared<ConnectionHandler>(
        serverAddress, serverPort, ConnectionHandlerType::Client, !debug
    );
    logger = LoggerFactory::getLogger("SegmentClient");
    logger->setLevel(debug ? LogLevel::Debug : LogLevel::Info);
}

SegmentClient::~SegmentClient() {
    stop();
}

void SegmentClient::start() {
    if (!connectionHandler->getIsWork()) {
        connectionHandler->start();
        logger->log(LogLevel::Info, "start", "Client handler started");
    }
}

void SegmentClient::stop() {
    logger->log(LogLevel::Info, "stop", "Stopping client...");
    disconnect();        // закрывает сокет и останавливает потоки чтения
    connectionHandler->stop(); // гарантированно останавливает внутренние потоки
    stopReadLoop();
    logger->log(LogLevel::Info, "stop", "Client stopped");
}

void SegmentClient::connect() {
    if (!connectionHandler->getIsWork()) {
        start(); // если не стартовали, стартуем
    }
    auto sock = connectionHandler->connect();
    if (sock.ptr) {
        logger->log(LogLevel::Info, "connect", "Connected to " + serverAddress + ":" + std::to_string(serverPort));
        if (newHandler) newHandler();
        startReadLoop();
    } else {
        logger->log(LogLevel::Error, "connect", "Connection failed");
    }
}

void SegmentClient::disconnect() {
    stopReadLoop();
    if (connectionHandler->getIsWork()) {
        connectionHandler->disconnect(true);
        if (closeHandler) closeHandler();
        logger->log(LogLevel::Info, "disconnect", "Disconnected");
    }
}

void SegmentClient::write(const void* data, size_t sz) {
    auto sock = connectionHandler->getSocket();
    if (!sock.ptr || !sock.ptr->is_open()) {
        logger->log(LogLevel::Warn, "write", "Socket not open");
        return;
    }

    // Формируем JSON с массивом байт
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    boost::json::array arr;
    arr.reserve(sz);
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

    std::lock_guard<std::mutex> lock(writeMutex);
    TransportHandler transport(sock.ptr);
    if (!transport.write(tm)) {
        logger->log(LogLevel::Error, "write", "Failed to send data");
    }
}

void SegmentClient::runReadLoop() {
    auto sock = connectionHandler->getSocket();
    if (!sock.ptr || !sock.ptr->is_open()) {
        logger->log(LogLevel::Warn, "runReadLoop", "Socket not open, read loop aborted");
        return;
    }

    TransportHandler transport(sock.ptr, 0xA0ABA0A, debug);
    while (connectionHandler->getIsWork() && sock.ptr->is_open()) {
        TransportMessage tm = transport.read();
        if (tm.type.empty()) break; // соединение закрыто или ошибка

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
                    readHandler(buffer.data(), buffer.size());
                }
            }
        } catch (std::exception& e) {
            logger->log(LogLevel::Warn, "runReadLoop", "JSON parse error: " + std::string(e.what()));
        }
    }
    logger->log(LogLevel::Debug, "runReadLoop", "Read loop finished");
    if (closeHandler) closeHandler();
}

void SegmentClient::startReadLoop() {
    if (readThreadRunning) return;
    readThreadRunning = true;
    readThread = std::thread([this]() {
        runReadLoop();
        readThreadRunning = false;
    });
}

void SegmentClient::stopReadLoop() {
    if (readThreadRunning && readThread.joinable()) {
        // Ждём завершения потока чтения (он сам выйдет, когда isWork станет false)
        readThread.join();
        readThreadRunning = false;
    }
}

void SegmentClient::setCloseConnectionHandler(ConnChangeHandler h) {
    closeHandler = std::move(h);
    connectionHandler->setCloseConnectionHandler([this](ConnectedSocket /*sock*/) {
        if (closeHandler) closeHandler();
        stopReadLoop(); // дополнительная защита
    });
}

void SegmentClient::setNewConnectionHandler(ConnChangeHandler h) {
    newHandler = std::move(h);
    connectionHandler->setNewConnectionHandler([this](ConnectedSocket /*sock*/) {
        if (newHandler) newHandler();
    });
}

void SegmentClient::setReadHandler(ReadHandler h) {
    readHandler = std::move(h);
}