cmake_minimum_required(VERSION 3.21)

project(MessageHandler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system json)

file(GLOB SOURCES "source/*.cpp")

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(${PROJECT_NAME} PUBLIC
        Boost::system
        Boost::json
        Logger
        TransportHandler
)

add_subdirectory(tests)