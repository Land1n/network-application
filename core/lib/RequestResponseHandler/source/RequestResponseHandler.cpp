//
// Created by ivan on 31.03.2026.
//

#include "RequestResponseHandler.hpp"

RequestResponseHandlerBase::RequestResponseHandlerBase(const std::shared_ptr<CreatorMessage>& creator_message) :
    creator_message(creator_message)
{}