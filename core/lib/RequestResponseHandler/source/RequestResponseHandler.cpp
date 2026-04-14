//
// Created by ivan on 31.03.2026.
//

#include "RequestResponseHandler.hpp"

RequestResponseHandlerBase::RequestResponseHandlerBase(std::shared_ptr<CreatorMessage> creator_message, bool DEBUG)
    : creator_message(creator_message) {
    logger = LoggerFactory::getLogger("RequestResponseHandler");
    if (!DEBUG) logger->setLevel(LogLevel::Error);
}