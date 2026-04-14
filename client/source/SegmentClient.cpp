//
// Created by ivan on 14.03.2026.
//
#include "SegmentClient.hpp"

// SegmentClient::SegmentClient(const std::string& hostname, int port, bool debug, uint32_t reconnectTimeoutMs)
//     : hostname_(hostname), port_(port), debug_(debug), reconnectTimeoutMs_(reconnectTimeoutMs) {
//     connHandler_ = std::make_shared<ConnectionHandler>(hostname_, port_, ConnectionHandlerType::Client, !debug_);
//     logger_ = LoggerFactory::getLogger("SegmentClient");
//     if (debug_) logger_->setLevel(LogLevel::Debug);
//     else        logger_->setLevel(LogLevel::Info);
//     msgHandler_ = std::make_shared<MessageHandler>(debug_);
// }
//
// SegmentClient::~SegmentClient() {
//     stop();
// }
//
// void SegmentClient::start() {
//     if (isRunning_.load()) {
//         logger_->log(LogLevel::Warn, __func__, "Client already running");
//         return;
//     }
//     isRunning_.store(true);
//     connHandler_->start();  // инициализирует внутренние структуры
//     doConnect();            // первое подключение
// }
//
// void SegmentClient::doConnect() {
//     if (!isRunning_.load()) return;
//
//     logger_->log(LogLevel::Info, __func__, "Connecting to " + hostname_ + ":" + std::to_string(port_));
//     auto sock = connHandler_->connect(10);  // 10 попыток
//     if (sock) {
//         logger_->log(LogLevel::Info, __func__, "Connected");
//         if (newHandler) newHandler();   // уведомляем внешний код
//         // Запускаем поток чтения (если не запущен или перезапускаем)
//         if (readThread_ && readThread_->joinable()) {
//             readThread_->join();
//         }
//         readThread_ = std::make_shared<std::thread>(&SegmentClient::runReadLoop, this);
//     } else {
//         logger_->log(LogLevel::Error, __func__, "Failed to connect");
//         if (closeHandler) closeHandler(); // уведомляем об ошибке подключения
//         scheduleReconnect();
//     }
// }
//
// void SegmentClient::runReadLoop() {
//     auto socket = connHandler_->getSocket().ptr;
//     if (!socket || !socket->is_open()) {
//         logger_->log(LogLevel::Error, __func__, "No valid socket");
//         return;
//     }
//
//     TransportHandler transport(socket, 0xA0ABA0A, debug_);
//     logger_->log(LogLevel::Debug, __func__, "Read loop started");
//
//     while (isRunning_.load()) {
//         TransportMessage msg = transport.read();
//         if (msg.type.empty() && msg.payload.empty()) {
//             // соединение разорвано
//             logger_->log(LogLevel::Warn, __func__, "Connection lost");
//             break;
//         }
//
//         if (readHandler) {
//             readHandler(msg.payload.data(), msg.payload.size());
//         }
//     }
//
//     // Очистка после разрыва
//     if (isRunning_.load()) {
//         logger_->log(LogLevel::Info, __func__, "Connection lost, cleaning up");
//         if (closeHandler) closeHandler();
//         connHandler_->disconnect(false); // закрываем сокет, но не ждём поток
//         scheduleReconnect();
//     }
// }
//
// void SegmentClient::scheduleReconnect() {
//     if (!isRunning_.load()) return;
//     if (reconnectTimeoutMs_ == 0) {
//         logger_->log(LogLevel::Info, __func__, "Reconnect disabled, stopping client");
//         stop();
//         return;
//     }
//
//     logger_->log(LogLevel::Info, __func__, "Scheduling reconnect in " + std::to_string(reconnectTimeoutMs_) + " ms");
//     std::this_thread::sleep_for(std::chrono::milliseconds(reconnectTimeoutMs_));
//     if (isRunning_.load()) {
//         doConnect();
//     }
// }
//
// void SegmentClient::write(const void* data, size_t sz) {
//     if (!isRunning_.load()) {
//         logger_->log(LogLevel::Warn, __func__, "Client not running");
//         return;
//     }
//     auto socket = connHandler_->getSocket().ptr;
//     if (!socket || !socket->is_open()) {
//         logger_->log(LogLevel::Warn, __func__, "Socket not open");
//         disconnect();
//         return;
//     }
//
//     std::vector<uint8_t> payload(static_cast<const uint8_t*>(data),
//                                  static_cast<const uint8_t*>(data) + sz);
//     TransportMessage msg("raw", Transaction::Tests, std::move(payload));
//     TransportHandler transport(socket, 0xA0ABA0A, debug_);
//     bool ok = transport.write(msg);
//     if (!ok) {
//         logger_->log(LogLevel::Error, __func__, "Write failed, disconnecting");
//         disconnect();
//     }
// }
//
// void SegmentClient::disconnect() {
//     logger_->log(LogLevel::Debug, __func__, "Disconnecting");
//     if (connHandler_->getSocket().ptr && connHandler_->getSocket().ptr->is_open()) {
//         connHandler_->disconnect(true);
//     }
//     if (closeHandler) closeHandler();
// }
//
// void SegmentClient::stop() {
//     if (!isRunning_.load()) return;
//     isRunning_.store(false);
//     shouldReconnect_.store(false);
//
//     logger_->log(LogLevel::Info, __func__, "Stopping client");
//
//     disconnect();
//
//     if (readThread_ && readThread_->joinable()) {
//         readThread_->join();
//         readThread_.reset();
//     }
//
//     connHandler_->stop();
//     logger_->log(LogLevel::Info, __func__, "Client stopped");
// }
