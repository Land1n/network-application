//
// Created by ivan on 10.03.2026.
//
#pragma once

#include <complex>
#include <vector>
#include <string>
#include "message/message.h"
#include "message/transaction.h"
#include <boost/json.hpp>

namespace json = boost::json;

class SignalMessage : public Message {
public:
	SignalMessage(const std::string& type, Transaction transaction, int central_Freq,
	              std::vector<std::complex<float>> signal);
	SignalMessage(const std::string& type, Transaction transaction, json::value& jv);

	int getCentralFreq() const;
	std::vector<std::complex<float>> getSignal() const;

	json::object serialize() override;

private:
	int _central_Freq;
	std::vector<std::complex<float>> _signal;
};