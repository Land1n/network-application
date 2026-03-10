//
// Created by ivan on 10.03.2026.
//

#pragma once

#include <memory>
#include <functional>
#include <unordered_map>
#include <string>

#include "Message.hpp"
#include "InformationMessage.hpp"
#include "SignalMessage.hpp"

#include "boost/json.hpp"

namespace json = boost::json;


std::unique_ptr<Message> createMessage(const std::string& type,json::value &jv);