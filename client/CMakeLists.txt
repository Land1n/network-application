cmake_minimum_required(VERSION 3.21)
project(segment_client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/lib/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/*.h)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

target_include_directories(segment_client PUBLIC include)

target_link_libraries(segment_client PUBLIC
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

add_executable(client main.cpp)
target_link_libraries(client PRIVATE segment_client)