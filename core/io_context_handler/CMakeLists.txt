cmake_minimum_required(VERSION 3.21)

project(io_context_handler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Boost REQUIRED COMPONENTS system)

file(GLOB SOURCE_FILES "lib/*.cpp" "include/${PROJECT_NAME}/*.h")

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(${PROJECT_NAME} PRIVATE
        Boost::system
)

include("unittest.cmake")