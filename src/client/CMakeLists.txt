add_executable(client
    client.cpp
)

target_compile_features(client PRIVATE cxx_std_17)
target_include_directories(client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(client PRIVATE
    core_lib
    Boost::system
    Boost::json
)



