# add_subdirectory(include)

add_executable(chat_server
    main.cpp
    server.cpp
    client_session.cpp
)

target_compile_features(chat_server PRIVATE cxx_std_17)
target_include_directories(chat_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(chat_server PRIVATE
    Boost::system
)

find_package(Threads REQUIRED)
target_link_libraries(chat_server PRIVATE Threads::Threads)
