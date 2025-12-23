find_package(GTest REQUIRED)

add_executable(run_tests
    test_server.cpp
    test_request_response_handler.cpp
    ../server/server.cpp
    ../server/server_session.cpp
)

target_include_directories(run_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../server/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/include
)

target_link_libraries(run_tests PRIVATE
    GTest::gtest
    GTest::gtest_main
    Boost::system
    Boost::json
    core_lib
    Threads::Threads
)

enable_testing()
add_test(NAME ServerTests COMMAND run_tests)
