cmake_minimum_required(VERSION 3.14)
project(server_client_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(core/lib/ConnectionHandler)
add_subdirectory(core/lib/TransportHandler)
add_subdirectory(core/lib/MessageHandler)
add_subdirectory(core/lib/RequestResponseHandler)
add_subdirectory(core/lib/ThreadPool)
add_subdirectory(server)
add_subdirectory(client)

find_package(GTest REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread json)

file(GLOB_RECURSE TEST_SOURCES "core/lib/*/tests/*.cpp" "server/lib/*/tests/*.cpp")


add_executable(all_tests ${TEST_SOURCES})

file(GLOB_RECURSE INCLUDE_DIRS "core/lib/*/include" "server/lib/*/include")
target_include_directories(all_tests PRIVATE ${INCLUDE_DIRS})

target_link_libraries(all_tests PRIVATE
        GTest::gtest
        GTest::gtest_main
        Boost::system
        Boost::thread
        Boost::json
        ConnectionHandler
        TransportHandler
        MessageHandler
        ThreadPool
        ServerRequestResponseHandler
)

enable_testing()
add_test(NAME AllTests COMMAND all_tests)