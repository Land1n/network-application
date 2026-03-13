find_package(GTest REQUIRED)

project(ServerMessageHandlerTests)

file(GLOB SOURCES "*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE
        GTest::gtest
        GTest::gtest_main
        Boost::system
        Boost::json
        ServerMessageHandler
        ServerMessage
)

enable_testing()