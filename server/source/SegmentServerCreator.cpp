//
// Created by ivan on 22.04.2026.
//
#include "SegmentServerCreator.hpp"
#include "SegmentServer.hpp"

std::shared_ptr<Network::Server> SegmentServerCreator::create(const Network::ServerCreatorParams& params)
{
	return std::make_shared<SegmentServer>(params.port, params.multiConnect);
}