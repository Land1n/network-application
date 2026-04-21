find_package(GTest REQUIRED)

project(ConnectionHandlerTests)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g")
file(GLOB SOURCES "*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE
        GTest::gtest
        GTest::gtest_main
        Boost::system
        ConnectionHandler
#        ThreadPool
)

enable_testing()