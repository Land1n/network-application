//
// Created by guestuser on 28.05.2026.
//
#pragma once
#include <boost/asio.hpp>
#include "utils.hpp"

namespace ErrorHandler {
bool check_error(const error_code& ec, const std::string& function_name, bool viewInfo = false);
bool check_error(const std::exception& ec, const std::string& function_name);
}