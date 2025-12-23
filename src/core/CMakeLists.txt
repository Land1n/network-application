add_library(core_lib STATIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/request_response_handler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/request_response_handler.hpp
)


target_compile_features(core_lib PRIVATE cxx_std_17)

target_include_directories(core_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
)


target_link_libraries(core_lib PRIVATE
    Boost::json
)
