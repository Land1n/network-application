cmake_minimum_required(VERSION 3.21)

project(Server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)

file(GLOB LIB_SOURCES "include/*.hpp" "source/*.cpp")

add_library(ServerCore SHARED ${LIB_SOURCES})

target_include_directories(ServerCore PUBLIC include)

target_link_libraries(ServerCore PUBLIC
        Boost::system
        ConnectionHandler
        MessageHandler
        RequestResponseHandler
        TransportHandler
        ThreadPool
        ServerRequestResponseHandler
)

add_subdirectory(lib/ServerRequestResponseHandler)

add_executable(server_app main.cpp)

target_link_libraries(server_app PRIVATE
        ServerCore
)