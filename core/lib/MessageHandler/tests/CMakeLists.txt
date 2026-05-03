find_package(GTest REQUIRED)

project(MessageHandlerTests)
set(SOURCES ${PROJECT_NAME}.cpp main.cpp)

file(GLOB SOURCES "*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE
        GTest::gtest
        Boost::system
        Boost::json
        Logger
        TransportHandler
        MessageHandler
)

enable_testing()
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})