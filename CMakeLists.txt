cmake_minimum_required(VERSION 3.16)
project(ChatSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Boost 1.66 REQUIRED COMPONENTS system json)

add_subdirectory(src)

install(TARGETS chat_server chat_client
        RUNTIME DESTINATION bin
        )

#enable_testing()
