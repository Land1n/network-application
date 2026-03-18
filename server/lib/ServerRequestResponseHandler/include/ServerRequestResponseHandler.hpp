//
// Created by ivan on 10.03.2026.
//

#pragma once

#include "RequestResponseHandler.hpp"
#include "CreatorMessage.hpp"

class ServerRequestResponseHandler : public IRequestResponseHandler {
public:
    explicit ServerRequestResponseHandler(std::shared_ptr<CreatorMessage> creator_message)
        : IRequestResponseHandler(creator_message) {
    }

    std::unique_ptr<Message> processingRequestResponse(std::unique_ptr<Message>) override;
};
