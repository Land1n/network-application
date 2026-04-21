cmake_minimum_required(VERSION 3.21)
project(SegmentClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)

file(GLOB SOURCES "source/*.cpp")

add_subdirectory(lib/ClientRequestResponseHandler)

add_library(SegmentClientCore SHARED ${SOURCES})

target_include_directories(SegmentClientCore PUBLIC include)

target_link_libraries(SegmentClientCore PUBLIC
        Boost::system
        clientserveriface
        ConnectionHandler
        MessageHandler
        TransportHandler
        ClientRequestResponseHandler
        Logger
)

add_executable(seg_client_app main.cpp)

target_link_libraries(seg_client_app PRIVATE
        SegmentClientCore
)