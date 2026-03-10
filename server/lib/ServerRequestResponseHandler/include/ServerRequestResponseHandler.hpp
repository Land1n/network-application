//
// Created by ivan on 10.03.2026.
//

#pragma once

#include "RequestResponseHandler.hpp"

class ServerRequestResponseHandler : public IRequestResponseHandler {
public:
    std::unique_ptr<Message> processingRequestResponse(std::unique_ptr<Message>) override;
};
