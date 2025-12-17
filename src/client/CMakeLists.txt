add_executable(chat_client
    client.cpp
)

target_compile_features(chat_client PRIVATE cxx_std_17)
target_include_directories(chat_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(chat_client PRIVATE
    Boost::system
    Boost::json
)



