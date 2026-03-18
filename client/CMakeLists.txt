cmake_minimum_required(VERSION 3.21)

project(Client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)

file(GLOB LIB_SOURCES "include/*.hpp" "source/*.cpp")

add_library(ClientCore SHARED ${LIB_SOURCES})

target_include_directories(ClientCore PUBLIC include)

target_link_libraries(ClientCore PUBLIC
        Boost::system
        ConnectionHandler
        MessageHandler
#        RequestResponseHandler
        TransportHandler
        ThreadPool
)

add_executable(client_app main.cpp)

target_link_libraries(client_app PRIVATE
        ClientCore
)