//
// Created by ivan on 10.03.2026.
//

#include "MessageHandler.hpp"
#include "SignalMessage.hpp"
#include "InformationMessage.hpp"
#include "gtest/gtest.h"


GTEST_TEST(MessageHandlerTests, ValidParseSignal) {
    MessageHandler handler;
    std::string json_str = R"({
        "type": "signal",
        "transaction": 1,
        "central_Freq": 6100,
        "signal": [[-8.865925598144531E1, -6.549491882324219E1]]
    })";
    std::vector<uint8_t> payload(json_str.begin(), json_str.end());
    std::string type = "signal";
    TransportMessage transport_message(type, Transaction::Response, payload);
    std::unique_ptr<Message> message = handler.parse(transport_message);

    ASSERT_NE(message, nullptr);
    auto *signalMsg = dynamic_cast<SignalMessage *>(message.get());
    ASSERT_NE(signalMsg, nullptr);
    EXPECT_EQ(signalMsg->getCentralFreq(), 6100);
    EXPECT_EQ(signalMsg->getSignal().size(), 1);
}

//--gtest_repeat=100
TEST(MessageHandlerTests, ValidParseInformation) {
    MessageHandler handler;
    std::string json_str = R"({
        "type": "information",
        "transaction": 1,
        "numberCore": 4
    })";
    std::vector<uint8_t> payload(json_str.begin(), json_str.end());
    TransportMessage transport_message("information", Transaction::Response, payload);

    auto message = handler.parse(transport_message);
    ASSERT_NE(message, nullptr);

    auto *infoMsg = dynamic_cast<InformationMessage *>(message.get());
    ASSERT_NE(infoMsg, nullptr);
    EXPECT_EQ(infoMsg->getNumberCore(), 4);
}

TEST(MessageHandlerTests, ParseUnsupportedTypeReturnsNull) {
    MessageHandler handler;
    std::string json_str = R"({"some":"data"})";
    std::vector<uint8_t> payload(json_str.begin(), json_str.end());
    std::string type = "unsupported";
    TransportMessage transport_message(type, Transaction::Tests, payload);

    auto message = handler.parse(transport_message);
    EXPECT_EQ(message, nullptr);
}

TEST(MessageHandlerTests, ParseInvalidJsonReturnsNull) {
    MessageHandler handler;
    std::string json_str = R"({ malformed )";
    std::vector<uint8_t> payload(json_str.begin(), json_str.end());
    std::string type = "signal";
    TransportMessage transport_message(type, Transaction::Tests, payload);

    auto message = handler.parse(transport_message);
    EXPECT_EQ(message, nullptr);
}

TEST(MessageHandlerTests, SerializeValidSignal) {
    MessageHandler handler;
    std::vector<std::complex<float> > signal = {{-88.65925598144531f, -65.49491882324219f}};
    auto message = std::make_unique<SignalMessage>("signal", Transaction::Response, 6100, signal);

    TransportMessage transport = handler.serialize(std::move(message));
    EXPECT_EQ(transport.type, "signal");
    EXPECT_EQ(transport.transaction, Transaction::Response);
    EXPECT_FALSE(transport.payload.empty());

    auto parsed = handler.parse(transport);
    ASSERT_NE(parsed, nullptr);
    auto *signalParsed = dynamic_cast<SignalMessage *>(parsed.get());
    ASSERT_NE(signalParsed, nullptr);
    EXPECT_EQ(signalParsed->getCentralFreq(), 6100);
    EXPECT_EQ(signalParsed->getSignal().size(), 1);
}

TEST(MessageHandlerTests, SerializeValidInformation) {
    MessageHandler handler;
    // Исправленный вызов конструктора: добавили Transaction::Tests
    auto message = std::make_unique<InformationMessage>("information", Transaction::Tests, 8);

    TransportMessage transport = handler.serialize(std::move(message));
    EXPECT_EQ(transport.type, "information");
    EXPECT_EQ(transport.transaction, Transaction::Tests);
    EXPECT_FALSE(transport.payload.empty());

    auto parsed = handler.parse(transport);
    ASSERT_NE(parsed, nullptr);
    auto *infoParsed = dynamic_cast<InformationMessage *>(parsed.get());
    ASSERT_NE(infoParsed, nullptr);
    EXPECT_EQ(infoParsed->getNumberCore(), 8);
}

TEST(MessageHandlerTests, SerializeNullMessageReturnsEmpty) {
    MessageHandler handler;
    TransportMessage transport = handler.serialize(nullptr);
    EXPECT_TRUE(transport.payload.empty());
    EXPECT_TRUE(transport.type.empty());
}

TEST(MessageHandlerTests, SerializeUnsupportedTypeReturnsOnlyType) {
    MessageHandler handler;
    class UnknownMessage : public Message {
    public:
        UnknownMessage() : Message("unknown", Transaction::Tests) {}
    };
    auto message = std::make_unique<UnknownMessage>();
    std::string json_str = R"({"type":"unknown","transaction":-1})";
    std::vector<uint8_t> payload(json_str.begin(), json_str.end());
    TransportMessage transport = handler.serialize(std::move(message));
    std::cout << json_str << std::endl;

    EXPECT_EQ(transport.type, "unknown");
    EXPECT_EQ(transport.transaction, Transaction::Tests);
    EXPECT_EQ(transport.payload.size(), payload.size());
}
