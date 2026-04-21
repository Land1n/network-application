//
// Created by ivan on 22.04.2026.
//
#include "SegmentClientCreator.hpp"
#include "Logger.hpp"

std::shared_ptr<Network::Client> SegmentClientCreator::create(const Network::ClientCreatorParams& params) {
    bool debug = (params.logLevel >= static_cast<uint32_t>(LogLevel::Debug));
    return std::make_shared<SegmentClient>(params.hostname, params.port, debug);
}