//
// Created by ivan on 22.04.2026.
//

#pragma once

#include "clientserveriface/servercreator.h"
#include "segment_server/segment_server.h"
#include <memory>

class SegmentServerCreator : public Network::ServerCreator {
public:
	std::shared_ptr<Network::Server> create(const Network::ServerCreatorParams& params) override;
};