find_package(GTest REQUIRED)

project(TransportHandlerTests)

set(SOURCES ${PROJECT_NAME}.cpp main.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE
                GTest::gtest
                Boost::system
                Boost::json
                TransportHandler
                ConnectionHandler
)

enable_testing()
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})