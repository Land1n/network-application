cmake_minimum_required(VERSION 3.14)
project(server_client_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# add_compile_options(-fsanitize=address -g -static-libasan)
# add_link_options(-fsanitize=address -static-libasan)

set(Boost_USE_STATIC_RUNTIME OFF)

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

find_package(GTest REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread json chrono)

set(PARENT_PROJECT ON)

add_subdirectory(core/lib/IOContextHandler)
add_subdirectory(core/lib/ErrorHandler)
add_subdirectory(core/lib/Utils)
add_subdirectory(core/lib/Connection)
add_subdirectory(core/lib/Session)
add_subdirectory(core/lib/TransportHandler)
add_subdirectory(core/lib/Logger)
add_subdirectory(core/lib/MessageHandler)
# add_subdirectory(core/lib/RequestResponseHandler)
add_subdirectory(core/lib/ConfigurationHandler)
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
        IOContextHandler
        Utils
        Worker
        Logger
        Connection
        TransportHandler
        MessageHandler
        Session
)

enable_testing()
add_test(NAME AllTests COMMAND all_tests)

set(LIB_TARGETS
        IOContextHandler
        ErrorHandler
        Utils
        Connection
        Session
        TransportHandler
        Logger
        MessageHandler
        Worker
        ConfigurationHandler
        SegmentServerCore
        SegmentClientCore
)
install(TARGETS ${LIB_TARGETS}
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)

install(TARGETS seg_server_app seg_client_app
        RUNTIME DESTINATION bin
)
install(DIRECTORY DESTINATION config)