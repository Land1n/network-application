//
// Created by ivan on 12.05.2026.
//

#pragma once

#include "BaseTransportHandler.hpp"

#include <memory>

enum TypeTransportHandler {
	Sync,
	Async,
};

struct ParamsCreatorTransportHandler {
	std::shared_ptr<tcp::socket> socket;
	uint32_t magicNumber  = 0xA0ABA0A;
	TypeTransportHandler type;
};

class CreatorTransportHandler {
public:
	std::unique_ptr<BaseTransportHandler> create(const ParamsCreatorTransportHandler& params);
};