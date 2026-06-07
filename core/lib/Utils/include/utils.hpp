//
// Created by guestuser on 01.06.2026.
//

#pragma once

#include <boost/asio.hpp>
#include <boost/json.hpp>

using error_code = boost::system::error_code;
using tcp        = boost::asio::ip::tcp;
namespace json   = boost::json;

using CallBack = std::function<void(error_code)>;