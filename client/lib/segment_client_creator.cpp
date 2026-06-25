//
// Created by ivan on 22.04.2026.
//
#include "segment_client/segment_client_creator.h"
#include "logger/logger.h"

std::shared_ptr<Network::Client> SegmentClientCreator::create(const Network::ClientCreatorParams& params)
{
	return std::make_shared<SegmentClient>(params.hostname, params.port);
}