cmake_minimum_required(VERSION 3.21)

project(SegmentServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)

file(GLOB LIB_SOURCES "include/*.hpp" "source/*.cpp")
add_library(SegmentServerCore SHARED ${LIB_SOURCES})

target_include_directories(SegmentServerCore PUBLIC include)

target_link_libraries(SegmentServerCore PUBLIC
        Boost::system
        clientserveriface
        ConnectionHandler
        MessageHandler
        RequestResponseHandler
        TransportHandler
        ServerRequestResponseHandler
        Logger
)

add_subdirectory(lib/ServerRequestResponseHandler)

add_executable(seg_server_app main.cpp)

target_link_libraries(seg_server_app PRIVATE
        SegmentServerCore
)
add_subdirectory(tests)