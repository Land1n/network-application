cmake_minimum_required(VERSION 3.21)

project(connection)

find_package(Boost REQUIRED COMPONENTS system)

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/lib/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/*.h)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PUBLIC
        Boost::system
        io_context_handler
        logger
        utils
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

include("unittest.cmake")

