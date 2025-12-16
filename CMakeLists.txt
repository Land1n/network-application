cmake_minimum_required(VERSION 3.10)
project(server)                
find_package(Boost REQUIRED)


add_executable(server src/main.cpp src/server.cpp src/client_session.cpp)
add_executable(client src/client.cpp)

target_include_directories(server PRIVATE src/include)
