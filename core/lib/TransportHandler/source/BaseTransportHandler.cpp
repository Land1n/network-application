//
// Created by ivan on 12.05.2026.
//
#include "BaseTransportHandler.hpp"

BaseTransportHandler::BaseTransportHandler(const std::shared_ptr<tcp::socket>& socket, uint32_t magicNumber) :
    socket(socket), magicNumber(magicNumber)
{}

void BaseTransportHandler::setOnReadHandler(std::function<void(size_t, const void*, size_t)> handler)
{
	onReadHandler = handler;
}

void BaseTransportHandler::setOnWriteHandler(std::function<void(const void*, size_t)> handler)
{
	onWriteHandler = handler;
}
