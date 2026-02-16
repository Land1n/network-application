cmake_minimum_required(VERSION 3.21)

project(Server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Boost REQUIRED COMPONENTS system)

file(GLOB SOURCES "include/*.h" "source/*.cpp")


add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_link_libraries(${PROJECT_NAME} PUBLIC
    ThreadPool   
    ConnectionHandler  
    RequestResponseHandler     
    Boost::system
)

target_include_directories(${PROJECT_NAME} PUBLIC include)