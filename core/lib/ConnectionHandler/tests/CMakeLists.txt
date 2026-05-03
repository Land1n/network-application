find_package(GTest REQUIRED)

project(ConnectionHandlerTests)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g")
file(GLOB SOURCES "*.cpp")
set(SOURCES ${PROJECT_NAME}.cpp main.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE
        GTest::gtest
        GTest::gtest_main
        Boost::system
        ConnectionHandler
)

enable_testing()
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})