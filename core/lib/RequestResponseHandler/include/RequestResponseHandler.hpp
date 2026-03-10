//
// Created by ivan on 08.03.2026.
//

#pragma once
#include <memory>

#include "Message.hpp"

class IRequestResponseHandler {
public:
    virtual ~IRequestResponseHandler() = default;
    virtual std::unique_ptr<Message> processingRequestResponse(std::unique_ptr<Message>) = 0;
};