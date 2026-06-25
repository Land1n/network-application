cmake_minimum_required(VERSION 3.21)
project(clientserveriface)

file(GLOB_RECURSE  SOURCE_FILES   ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/*.h)
add_custom_target(${PROJECT_NAME}_headers SOURCES ${SOURCE_FILES})
add_library(${PROJECT_NAME} INTERFACE) 
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

