cmake_minimum_required(VERSION 3.21)

project(utils)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Boost REQUIRED COMPONENTS system json)

file(GLOB SOURCE_FILES "lib/*.cpp" "include/utils/*.h")

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(${PROJECT_NAME} PRIVATE
        Boost::system
        Boost::json
        logger
)

include("unittest.cmake")