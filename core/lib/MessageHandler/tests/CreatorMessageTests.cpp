//
// Created by ivan on 11.03.2026.
//

#include <gtest/gtest.h>

#include "CreatorMessage.hpp"

class MessageTest : public Message {
public:
    MessageTest(const std::string &type, Transaction transaction, json::value &jv) : Message(type, transaction) {}
    json::object serialize() {return json::object();}
};

GTEST_TEST(CreatorMessageTests, CreateValidMessage) {
    CreatorMessage creator({
        {"test", [](const std::string& t, Transaction tr, json::value& v) {
            return std::make_unique<MessageTest>(t, tr, v);
        }}
    });
    json::value jv;
    std::string type = "test";
    auto message = creator.createMessage(type, Transaction::Tests, jv);
    ASSERT_NE(message, nullptr);
    EXPECT_EQ(message->type, "test");
    EXPECT_EQ(message->transaction, Transaction::Tests);
}

GTEST_TEST(CreatorMessageTests, AddMessageOnMap) {
    CreatorMessage creator;
    json::value jv;
    std::string type = "test";
    creator.addMessageOnMap(type, [](const std::string& t, Transaction tr, json::value& v) {
        return std::make_unique<MessageTest>(t, tr, v);
    });
    auto message = creator.createMessage(type, Transaction::Tests, jv);
    ASSERT_NE(message, nullptr);
    EXPECT_EQ(message->type, "test");
}

GTEST_TEST(CreatorMessageTests, CreateUnsupportedMessage) {
    CreatorMessage creator;
    json::value jv;
    std::string type = "test";
    auto message = creator.createMessage(type, Transaction::Tests, jv);
    EXPECT_EQ(message, nullptr);
}