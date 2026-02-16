cmake_minimum_required(VERSION 3.21)

project(server_app VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON) 

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Boost REQUIRED COMPONENTS system thread)

add_subdirectory(src/core/lib/ThreadPool)
add_subdirectory(src/server/lib/ConnectionHandler)
add_subdirectory(src/server/lib/RequestResponseHandler)
add_subdirectory(src/server/app)

add_executable(server_app main.cpp)

target_link_libraries(server_app PUBLIC
    Server                 
    Boost::system
)

target_include_directories(server_app PRIVATE src/server/app/include)

install(TARGETS ConnectionHandler 
        RUNTIME DESTINATION bin
        )