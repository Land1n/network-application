#include <gtest/gtest.h>
#include "../core/include/request_response_handler.hpp"

TEST(RequestResponseHandlerTest, ParseValidData) {
    RequestResponseHandler handler;
    
    // Тест разбора валидных JSON данных
    const char* json_data = R"({"command":"ping","data":[1,2,3]})";
    MessageData result = handler.parseData(json_data);
    
    EXPECT_EQ(result.command, "ping");
    EXPECT_EQ(result.data.size(), 3);
    EXPECT_EQ(result.data[0], 1);
    EXPECT_EQ(result.data[1], 2);
    EXPECT_EQ(result.data[2], 3);
}

TEST(RequestResponseHandlerTest, SerializeMessageData) {
    RequestResponseHandler handler;
    
    // Тест сериализации данных в JSON
    MessageData message("test_command", {10, 20, 30});
    std::string json_result = handler.serializeData(message);
    
    // Проверяем, что результат содержит ожидаемые поля
    EXPECT_TRUE(json_result.find("\"command\":\"test_command\"") != std::string::npos);
    EXPECT_TRUE(json_result.find("\"data\":[10,20,30]") != std::string::npos);
}

TEST(RequestResponseHandlerTest, ProcessPingCommand) {
    RequestResponseHandler handler;
    
    // Тест обработки команды "ping"
    const char* ping_data = R"({"command":"ping","data":[5]})";
    MessageData result = handler.processingCommand(ping_data);
    
    EXPECT_EQ(result.command, "pong");
    EXPECT_EQ(result.data.size(), 1);
    EXPECT_EQ(result.data[0], 6); // 5 + 1
}

TEST(RequestResponseHandlerTest, ProcessGetDataCommand) {
    RequestResponseHandler handler;
    
    // Тест обработки команды "getData"
    const char* getdata_json = R"({"command":"getData","data":[100]})";
    MessageData result = handler.processingCommand(getdata_json);
    
    EXPECT_EQ(result.command, "sendData");
    EXPECT_EQ(result.data.size(), 1);
    EXPECT_EQ(result.data[0], 0);
}

TEST(RequestResponseHandlerTest, ProcessUnknownCommand) {
    RequestResponseHandler handler;
    
    // Тест обработки неизвестной команды
    const char* unknown_json = R"({"command":"unknown","data":[1,2,3]})";
    MessageData result = handler.processingCommand(unknown_json);
    
    EXPECT_EQ(result.command, "error");
    EXPECT_EQ(result.data.size(), 1);
    EXPECT_EQ(result.data[0], -1);
}
