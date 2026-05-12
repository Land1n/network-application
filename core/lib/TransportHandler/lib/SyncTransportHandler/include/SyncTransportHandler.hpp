//
// Created by ivan on 12.05.2026.
//
#pragma once

#include "BaseTransportHandler.hpp"

class SyncTransportHandler : public BaseTransportHandler {
public:
	SyncTransportHandler(std::shared_ptr<tcp::socket> socket, uint32_t magicNumber);
	void read(TransportMessage& in_message,boost::system::error_code& code) override;
	void write(const TransportMessage& out_message,boost::system::error_code& code) override;
};