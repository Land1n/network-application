cmake_minimum_required(VERSION 3.14)
project(server_client_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread json chrono)

set(PARENT_PROJECT ON)

add_subdirectory(core/lib/ConnectionHandler)
add_subdirectory(core/lib/TransportHandler)
add_subdirectory(core/lib/Logger)
add_subdirectory(core/lib/MessageHandler)
add_subdirectory(core/lib/RequestResponseHandler)
add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(clientserveriface)
add_subdirectory(core/lib/Worker)


file(GLOB_RECURSE TEST_SOURCES
        "core/lib/*/tests/*.cpp"
        "server/lib/*/tests/*.cpp"
        "server/tests/*.cpp"
        "client/tests/*.cpp"
)
list(FILTER TEST_SOURCES EXCLUDE REGEX ".*/main\\.cpp$")


add_executable(all_tests ${TEST_SOURCES})

file(GLOB_RECURSE INCLUDE_DIRS
        "core/lib/*/include"
        "server/lib/*/include"
        "server/include"
        "client/include"
)
target_include_directories(all_tests PRIVATE ${INCLUDE_DIRS})

target_link_libraries(all_tests PRIVATE
        GTest::gtest
        GTest::gtest_main
        Boost::system
        Boost::thread
        Boost::json
        Boost::chrono
        Worker
        Logger
        ConnectionHandler
        TransportHandler
        MessageHandler
        ServerRequestResponseHandler
        SegmentServerCore
)

enable_testing()
add_test(NAME AllTests COMMAND all_tests)