cmake_minimum_required(VERSION 3.21)

project(RequestResponseHandler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE include)

target_link_libraries(${PROJECT_NAME} INTERFACE
        Boost::system
)