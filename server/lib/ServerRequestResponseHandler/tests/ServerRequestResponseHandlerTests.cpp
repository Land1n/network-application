//
// Created by ivan on 13.03.2026.
//

#include "ServerRequestResponseHandler.hpp"

#include "gtest/gtest.h"

#include "SignalMessage.hpp"
#include "InformationMessage.hpp"

#include <memory>
#include <complex>
#include <vector>

GTEST_TEST(ServerRequestResponseHandlerTests, SignalRequest_ReturnsSignalResponseWithCorrectData) {
    auto creator = std::make_shared<CreatorMessage>();
    creator->addMessageOnMap("signal", [](const std::string& type, boost::json::value& jv) -> std::unique_ptr<Message> {
        return std::make_unique<SignalMessage>(type, jv);
    });
    ServerRequestResponseHandler handler(creator);

    auto request = std::make_unique<Message>("signal");
    request->transactionType = TransactionType::Request;

    auto response = handler.processingRequestResponse(std::move(request));
    ASSERT_NE(response, nullptr);
    EXPECT_EQ(response->type, "signal");
    EXPECT_EQ(response->transactionType, TransactionType::Response);

    auto* signalMsg = dynamic_cast<SignalMessage*>(response.get());
    ASSERT_NE(signalMsg, nullptr);
    EXPECT_EQ(signalMsg->getCentralFreq(), 6100);

    auto signal = signalMsg->getSignal();
    ASSERT_EQ(signal.size(), 1);
    EXPECT_FLOAT_EQ(signal[0].real(), -8.865925598144531E1f);
    EXPECT_FLOAT_EQ(signal[0].imag(), -6.549491882324219E1f);
}

GTEST_TEST(ServerRequestResponseHandlerTests, InformationRequest_ReturnsInformationResponseWithCorrectData) {
    auto creator = std::make_shared<CreatorMessage>();
    creator->addMessageOnMap("information", [](const std::string& type, boost::json::value& jv) -> std::unique_ptr<Message> {
        return std::make_unique<InformationMessage>(type, jv);
    });
    ServerRequestResponseHandler handler(creator);

    auto request = std::make_unique<Message>("information");
    request->transactionType = TransactionType::Request;

    auto response = handler.processingRequestResponse(std::move(request));
    ASSERT_NE(response, nullptr);
    EXPECT_EQ(response->type, "information");
    EXPECT_EQ(response->transactionType, TransactionType::Response);

    auto* infoMsg = dynamic_cast<InformationMessage*>(response.get());
    ASSERT_NE(infoMsg, nullptr);
    EXPECT_EQ(infoMsg->getNumberCore(), 4);
}

GTEST_TEST(ServerRequestResponseHandlerTests, UnknownRequestType_ReturnsNullptr) {
    auto creator = std::make_shared<CreatorMessage>();
    ServerRequestResponseHandler handler(creator);

    auto request = std::make_unique<Message>("unknown");
    request->transactionType = TransactionType::Request;

    auto response = handler.processingRequestResponse(std::move(request));
    EXPECT_EQ(response, nullptr);
}