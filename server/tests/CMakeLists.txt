find_package(GTest REQUIRED)

project(SegmentServerTests)

file(GLOB SOURCES "*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE
        GTest::gtest
        GTest::gtest_main
        Boost::system
        Boost::json
        SegmentServer
        TransportHandler
        MessageHandler
        ServerRequestResponseHandler
        ConnectionHandler
)

enable_testing()