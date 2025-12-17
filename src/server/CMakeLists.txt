# add_subdirectory(include)

add_executable(server
    main.cpp
    server.cpp
    server_session.cpp
)

target_compile_features(server PRIVATE cxx_std_17)
target_include_directories(server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}

)

find_package(Threads REQUIRED)
target_link_libraries(server PRIVATE
    Boost::system
    Boost::json 
    Threads::Threads
)

