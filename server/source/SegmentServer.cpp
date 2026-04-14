//
// Created by ivan on 09.03.2026.
//

#include "SegmentServer.hpp"
#include "TransportHandler.hpp"

SegmentServer::SegmentServer(const std::string& address, int port,bool debug)
    : address(address), port(port) {
    connectionHandler = std::make_shared<ConnectionHandler>(
        address, port, ConnectionHandlerType::Server, !debug
        );
    logger = LoggerFactory::getLogger("SegmentServer");
    if (debug) logger->setLevel(LogLevel::Debug);
    else        logger->setLevel(LogLevel::Info);

    // Initialize message handlers (even if auto processing is disabled, they may be used later)
    // msgHandler_ = std::make_shared<MessageHandler>(debug_);
    // rrHandler_ = std::make_shared<ServerRequestResponseHandler>(msgHandler_->creator_message);
}
//
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
//
// void SegmentServer::start() {
//     if (isRunning_.load()) {
//         logger_->log(LogLevel::Warn, __func__, "Server already running");
//         return;
//     }
//
//     connHandler_->start();
//     connHandler_->listen();  // starts acceptor_thread and connection_check_thread
//
//     // Set task socket callback – called for each new accepted connection
//     connHandler_->setTaskSocket([this](std::shared_ptr<tcp::socket> sock) {
//         // Generate or obtain connection ID
//         Network::ConnectionId id;
//         if (idDistHandler_) {
//             id = idDistHandler_();
//         } else {
//             id = nextId_.fetch_add(1);
//         }
//
//         // Store connection context
//         ConnectionContext ctx;
//         ctx.socket = sock;
//         ctx.stopReading = false;
//         {
//             std::lock_guard<std::mutex> lock(connectionsMutex_);
//             connections_[id] = ctx;
//         }
//
//         // Notify user about new connection
//         if (newHandler) {
//             newHandler(id);
//         }
//
//         // Start reading thread for this connection
//         startReading(id, sock);
//     });
//
//     isRunning_.store(true);
//     logger_->log(LogLevel::Info, __func__, "Server started on " + address_ + ":" + std::to_string(port_));
// }
//
// void SegmentServer::startReading(Network::ConnectionId id, std::shared_ptr<tcp::socket> socket) {
//     auto readThread = std::make_shared<std::thread>([this, id, socket]() {
//         TransportHandler transport(socket, 0xA0ABA0A, debug_);
//         logger_->log(LogLevel::Debug, __func__, "Reading thread started for id=" + std::to_string(id));
//
//         while (true) {
//             {
//                 std::lock_guard<std::mutex> lock(connectionsMutex_);
//                 auto it = connections_.find(id);
//                 if (it == connections_.end() || it->second.stopReading) {
//                     break;
//                 }
//             }
//
//             TransportMessage msg = transport.read();
//             if (msg.type.empty() && msg.payload.empty()) {
//                 // Connection closed or error
//                 logger_->log(LogLevel::Debug, __func__, "Read failed or connection closed for id=" + std::to_string(id));
//                 break;
//             }
//
//             bool handled = false;
//             if (autoHandleRequests_) {
//                 auto message = msgHandler_->parse(msg);
//                 if (message && message->transaction == Transaction::Request) {
//                     auto response = rrHandler_->processingRequestResponse(std::move(message));
//                     if (response) {
//                         TransportMessage respMsg = msgHandler_->serialize(std::move(response));
//                         if (transport.write(respMsg)) {
//                             logger_->log(LogLevel::Debug, __func__, "Auto-response sent for id=" + std::to_string(id));
//                             handled = true;
//                         } else {
//                             logger_->log(LogLevel::Warn, __func__, "Failed to send auto-response for id=" + std::to_string(id));
//                         }
//                     }
//                 }
//             }
//
//             if (!handled && readHandler) {
//                 readHandler(id, msg.payload.data(), msg.payload.size());
//             }
//         }
//
//         onConnectionClosed(id);
//     });
//
//     {
//         std::lock_guard<std::mutex> lock(connectionsMutex_);
//         auto it = connections_.find(id);
//         if (it != connections_.end()) {
//             it->second.readThread = readThread;
//         }
//     }
// }
//
// void SegmentServer::onConnectionClosed(Network::ConnectionId id) {
//     logger_->log(LogLevel::Debug, __func__, "Cleaning up connection id=" + std::to_string(id));
//
//     std::shared_ptr<tcp::socket> socket;
//     {
//         std::lock_guard<std::mutex> lock(connectionsMutex_);
//         auto it = connections_.find(id);
//         if (it == connections_.end()) return;
//         socket = it->second.socket;
//         connections_.erase(it);
//     }
//
//     // Close socket if still open
//     if (socket && socket->is_open()) {
//         boost::system::error_code ec;
//         socket->close(ec);
//         if (ec) {
//             logger_->log(LogLevel::Warn, __func__, "Error closing socket: " + ec.message());
//         }
//     }
//
//     // Notify user
//     if (closeHandler) {
//         closeHandler(id);
//     }
// }
//
// void SegmentServer::stop() {
//     if (!isRunning_.load()) return;
//     isRunning_.store(false);
//
//     logger_->log(LogLevel::Info, __func__, "Stopping server...");
//
//     std::vector<Network::ConnectionId> ids;
//     {
//         std::lock_guard<std::mutex> lock(connectionsMutex_);
//         for (auto& pair : connections_) {
//             pair.second.stopReading = true;
//             ids.push_back(pair.first);
//         }
//     }
//
//     for (auto id : ids) {
//         std::shared_ptr<std::thread> thr;
//         {
//             std::lock_guard<std::mutex> lock(connectionsMutex_);
//             auto it = connections_.find(id);
//             if (it != connections_.end() && it->second.readThread) {
//                 thr = it->second.readThread;
//             }
//         }
//         if (thr && thr->joinable()) {
//             thr->join();
//         }
//     }
//
//     {
//         std::lock_guard<std::mutex> lock(connectionsMutex_);
//         connections_.clear();
//     }
//
//     connHandler_->stop();
//
//     logger_->log(LogLevel::Info, __func__, "Server stopped");
// }
//
// void SegmentServer::write(Network::ConnectionId id, const void* data, size_t sz) {
//     std::shared_ptr<tcp::socket> socket;
//     {
//         std::lock_guard<std::mutex> lock(connectionsMutex_);
//         auto it = connections_.find(id);
//         if (it == connections_.end()) {
//             logger_->log(LogLevel::Warn, __func__, "Unknown connection id=" + std::to_string(id));
//             return;
//         }
//         socket = it->second.socket;
//     }
//
//     if (!socket || !socket->is_open()) {
//         logger_->log(LogLevel::Warn, __func__, "Socket not open for id=" + std::to_string(id));
//         disconnect(id);
//         return;
//     }
//
//     // Pack raw data into TransportMessage
//     std::vector<uint8_t> payload(static_cast<const uint8_t*>(data),
//                                  static_cast<const uint8_t*>(data) + sz);
//     TransportMessage msg("raw", Transaction::Tests, std::move(payload));
//
//     TransportHandler transport(socket, 0xA0ABA0A, debug_);
//     bool ok = transport.write(msg);
//     if (!ok) {
//         logger_->log(LogLevel::Error, __func__, "Write failed for id=" + std::to_string(id));
//         disconnect(id);
//     }
// }
//
// void SegmentServer::disconnect(Network::ConnectionId id) {
//     logger_->log(LogLevel::Debug, __func__, "Disconnecting id=" + std::to_string(id));
//
//     {
//         std::lock_guard<std::mutex> lock(connectionsMutex_);
//         auto it = connections_.find(id);
//         if (it != connections_.end()) {
//             it->second.stopReading = true;
//             if (it->second.socket && it->second.socket->is_open()) {
//                 boost::system::error_code ec;
//                 it->second.socket->close(ec);
//             }
//         }
//     }
//
//     std::shared_ptr<std::thread> thr;
//     {
//         std::lock_guard<std::mutex> lock(connectionsMutex_);
//         auto it = connections_.find(id);
//         if (it != connections_.end() && it->second.readThread) {
//             thr = it->second.readThread;
//         }
//     }
//     if (thr && thr->joinable()) {
//         thr->join();
//     }
//
// }