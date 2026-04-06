#pragma once

#include "RequestResponseHandler.hpp"
#include "CreatorMessage.hpp"

class ClientRequestResponseHandler : public RequestResponseHandlerBase {
public:
    explicit ClientRequestResponseHandler(std::shared_ptr<CreatorMessage> creator_message)
        : RequestResponseHandlerBase(creator_message) {
    }

    std::unique_ptr<Message> processingRequestResponse(std::unique_ptr<Message>) override;
};
