cmake_minimum_required(VERSION 3.21)

project(transport_handler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Boost REQUIRED COMPONENTS system json)
find_package(GTest REQUIRED)

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/lib/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/*.h)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PUBLIC
        Boost::system
        Boost::json
        message
        utils
        connection
        io_context_handler
        message_handler
        logger
        clientserveriface
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

include("unittest.cmake")