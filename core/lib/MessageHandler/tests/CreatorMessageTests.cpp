//
// Created by ivan on 11.03.2026.
//

#include <gtest/gtest.h>

#include "CreatorMessage.hpp"

class MessageTest : public Message {
    public:
    MessageTest(const std::string &type,json::value &jv) : Message(type) {}
};

GTEST_TEST(CreatorMessageTests, CreateValidMessage) {
    CreatorMessage creator({
        {"test", [](const std::string& t, json::value& v) {
            return std::make_unique<MessageTest>(t, v);
        }}
    });
    json::value jv;
    std::string type = "test";
    auto message = creator.createMessage(type, jv);
    ASSERT_NE(message, nullptr);
    EXPECT_EQ(message->type, "test");
}

GTEST_TEST(CreatorMessageTests, AddMessageOnMap) {
    CreatorMessage creator;
    json::value jv;
    std::string type = "test";
    creator.addMessageOnMap(type,[](const std::string& t, json::value& v) { return std::make_unique<MessageTest>(t, v); });
    auto message = creator.createMessage(type,jv);
    ASSERT_NE(message, nullptr);
    EXPECT_EQ(message->type, "test");
}

GTEST_TEST(CreatorMessageTests, CreateUnsupportedMessage) {
    CreatorMessage creator;
    json::value jv;
    std::string type = "test";
    auto message = creator.createMessage(type,jv);
    EXPECT_EQ(message, nullptr);
}