//
// Created by ivan on 22.04.2026.
//

#include "segment_server/segment_server_creator.h"
#include "segment_server/segment_server.h"

std::shared_ptr<Network::Server> SegmentServerCreator::create(const Network::ServerCreatorParams& params)
{
	return std::make_shared<SegmentServer>(params.port, params.multiConnect);
}