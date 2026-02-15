cmake_minimum_required(VERSION 3.21)

project(ChatSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON) 

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Boost REQUIRED COMPONENTS system thread)


add_subdirectory(src/core/lib/ThreadPool)
add_subdirectory(src/server/lib/ConnectionHandler)
add_subdirectory(src/server/app)    

install(TARGETS ConnectionHandler 
        RUNTIME DESTINATION bin
        )