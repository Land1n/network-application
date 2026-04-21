//
// Created by ivan on 22.04.2026.
//

#pragma once

#include "clientserveriface/clientcreator.h"
#include "SegmentClient.hpp"
#include <memory>

class SegmentClientCreator : public Network::ClientCreator {
public:
    std::shared_ptr<Network::Client> create(const Network::ClientCreatorParams& params) override;
};