cmake_minimum_required(VERSION 3.21)

project(segment_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system json)

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/lib/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/*.h)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(${PROJECT_NAME} PUBLIC
        Boost::system
        clientserveriface
        message
        session
        configuration_handler
        worker
        request_response_handler
        logger
        utils
)

add_executable(server main.cpp)
target_link_libraries(server PRIVATE segment_server)

include("unittest.cmake")