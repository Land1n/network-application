//
// Created by guestuser on 28.05.2026.
//
#pragma once
#include <boost/asio.hpp>

using error_code = boost::system::error_code;


namespace ErrorHandler {
bool check_error(const error_code& ec, const std::string& function_name);
bool check_error(const std::exception& ec, const std::string& function_name);
}