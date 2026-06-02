cmake_minimum_required(VERSION 3.21)
project(Utils)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE include)

# Propagate Boost dependency to any target that links Utils
target_link_libraries(${PROJECT_NAME} INTERFACE Boost::system)