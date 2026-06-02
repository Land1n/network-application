//
// Created by guestuser on 01.06.2026.
//

#pragma once

#include <boost/asio.hpp>
#include <boost/json.hpp>

enum class IOMode {
	Sync = 0,
	Async = 1,
};


using error_code = boost::system::error_code;
using tcp = boost::asio::ip::tcp;
namespace json = boost::json;

using CallBack = std::function<void(error_code)>;

inline std::string getSyncOrAsync( IOMode type)
{
	if(type == IOMode::Sync) {
		return "sync";
	}
	return "async";
}

