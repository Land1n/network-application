#pragma once

#include "RequestResponseHandler.hpp"

#include <utility>
#include "CreatorMessage.hpp"

class ClientRequestResponseHandler : public RequestResponseHandlerBase {
public:
	explicit ClientRequestResponseHandler(const std::shared_ptr<CreatorMessage>& creator_message);
	std::unique_ptr<Message> processingRequestResponse(const std::unique_ptr<Message>& ) override;
};
