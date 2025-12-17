find_package(Threads REQUIRED)

# Создаем библиотеку из серверных файлов
add_library(server_lib STATIC
    server.cpp
    server_session.cpp
)

target_compile_features(server_lib PRIVATE cxx_std_17)
target_include_directories(server_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(server_lib PRIVATE
    Boost::system
    Boost::json 
    Threads::Threads
)

# Создаем исполняемый файл сервера
add_executable(server
    main.cpp
)

target_include_directories(server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(server PRIVATE
    server_lib
    Boost::system
    Boost::json
    Threads::Threads
)
