cmake_minimum_required(VERSION 3.21)

project(TransportHandler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Boost REQUIRED COMPONENTS system json)

# List all core sources except the factory
set(CORE_SOURCES
        source/BaseTransportHandler.cpp
        source/TransportHandler.cpp
        source/Transaction.cpp
)

# Optional: also list the corresponding headers for dependency tracking
set(CORE_HEADERS
        include/BaseTransportHandler.hpp
        include/TransportHandler.hpp
        include/TransportMessage.hpp
        include/Transaction.hpp
)

add_library(TransportHandlerCore SHARED ${CORE_SOURCES} ${CORE_HEADERS})

target_link_libraries(TransportHandlerCore PUBLIC
        Boost::system
        Boost::json
        Logger
)
target_include_directories(TransportHandlerCore PUBLIC include)

# Build the SyncTransportHandler subproject
add_subdirectory(lib/SyncTransportHandler)

# Factory library – knows about concrete handlers
add_library(TransportHandlerFactory STATIC
        source/CreatorTransportHandler.cpp
)
target_link_libraries(TransportHandlerFactory PUBLIC
        TransportHandlerCore
        SyncTransportHandler
)
target_include_directories(TransportHandlerFactory PUBLIC include)

# Convenience aggregate for external consumers (like the tests)
add_library(TransportHandler INTERFACE)
target_link_libraries(TransportHandler INTERFACE
        TransportHandlerCore
        SyncTransportHandler
        TransportHandlerFactory
)

add_subdirectory(tests)