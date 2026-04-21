//
// Created by ivan on 22.04.2026.
//
#include "SegmentServerCreator.hpp"
#include "SegmentServer.hpp"
#include "Logger.hpp"

std::shared_ptr<Network::Server> SegmentServerCreator::create(const Network::ServerCreatorParams& params) {
    // Устанавливаем флаг отладки, если уровень логирования >= Debug
    bool debug = (params.logLevel >= static_cast<uint32_t>(LogLevel::Debug));
    // Адрес по умолчанию (можно вынести в параметры при необходимости)
    std::string address = "127.0.0.1";
    return std::make_shared<SegmentServer>(address, params.port, params.multiConnect, debug);
}