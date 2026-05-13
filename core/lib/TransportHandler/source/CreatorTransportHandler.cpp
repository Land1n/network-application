//
// Created by ivan on 12.05.2026.
//
#include "CreatorTransportHandler.hpp"
#include "SyncTransportHandler.hpp"
#include "AsyncTransportHandler.hpp"

std::unique_ptr<BaseTransportHandler> CreatorTransportHandler::create(const ParamsCreatorTransportHandler& params)
{
	if (params.socket != nullptr) {
		if(params.type == TypeTransportHandler::Sync)
			return std::make_unique<SyncTransportHandler>(params.socket, params.magicNumber);
		else if(params.type == TypeTransportHandler::Async)
			return std::make_unique<AsyncTransportHandler>(params.socket, params.magicNumber);
	}

	return nullptr;
}