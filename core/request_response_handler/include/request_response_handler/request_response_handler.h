//
// Created by guestuser on 21.06.2026.
//

#ifndef SERVER_CLIENT_APP_REQUEST_RESPONSE_HANDLER_H
#define SERVER_CLIENT_APP_REQUEST_RESPONSE_HANDLER_H

#include "message/message.h"
#include <memory>
namespace RequestResponseHandler {
std::unique_ptr<Message> createRequest(std::string&& type_message);
std::unique_ptr<Message> createResponse(std::unique_ptr<Message>&& message);
}; // namespace RequestResponseHandler

#endif // SERVER_CLIENT_APP_REQUEST_RESPONSE_HANDLER_H
