find_package(Threads REQUIRED)


add_executable(server
    main.cpp
    server.cpp
    server_session.cpp
)

target_include_directories(server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/include
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(server PRIVATE
    	core_lib
	Boost::system
	Threads::Threads
)
