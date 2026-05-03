find_package(GTest REQUIRED)

project(WorkerTests)
set(SOURCES ${PROJECT_NAME}.cpp main.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE
        GTest::gtest
        Worker
        Logger
)

enable_testing()

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})