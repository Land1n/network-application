//
// Created by guestuser on 22.06.2026.
//
#pragma once

#include "utils/alias.h" // для error_code и std::string
#include <string>
#include <exception>

namespace ErrorHandler {
bool check_error(const error_code& ec, const std::string& function_name, bool viewInfo = false);
bool check_error(const std::exception& ec, const std::string& function_name);
} // namespace ErrorHandler