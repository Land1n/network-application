project(server_app)

add_executable(server_app main.cpp)

target_link_libraries(server_app PRIVATE ConnectionHandler ThreadPool)
target_include_directories(server_app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})


install(TARGETS server_app RUNTIME DESTINATION bin)