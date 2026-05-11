//
// Created by ivan on 22.04.2026.
//
#include "SegmentServerCreator.hpp"
#include "SegmentServer.hpp"
#include "Logger.hpp"

std::shared_ptr<Network::Server> SegmentServerCreator::create(const Network::ServerCreatorParams& params) {
    std::string address = "127.0.0.1";
    return std::make_shared<SegmentServer>(address, params.port, params.multiConnect);
}