cmake_minimum_required(VERSION 3.21)

project(RequestResponseHandler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)

file(GLOB SOURCES "source/*.cpp")

#add_library(${PROJECT_NAME} INTERFACE)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(${PROJECT_NAME} PUBLIC
        Boost::system
        Logger
        MessageHandler
)