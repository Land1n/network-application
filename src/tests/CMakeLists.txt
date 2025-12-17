find_package(GTest REQUIRED)

add_executable(run_tests
    test_server.cpp
)

target_include_directories(run_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../server/include
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(run_tests PRIVATE
    GTest::gtest
    GTest::gtest_main
    Boost::system
    Boost::json
    server_lib
)


enable_testing()
add_test(NAME ServerTests COMMAND run_tests)
