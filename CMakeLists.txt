cmake_minimum_required(VERSION 3.21)
project(network-application)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CMAKE_SKIP_BUILD_RPATH FALSE)                # Не пропускать RPATH при сборке
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)         # Использовать установочный RPATH уже на этапе сборки
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")        # Путь к библиотекам относительно исполняемого файла
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)      # Добавлять пути зависимостей в RPATH
set(UNITTEST ON)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


add_subdirectory(core/connection)
add_subdirectory(core/io_context_handler)
add_subdirectory(core/logger)
add_subdirectory(core/utils)
add_subdirectory(core/worker)
add_subdirectory(core/session)
add_subdirectory(core/message)
add_subdirectory(core/message_handler)
add_subdirectory(core/transport_handler)
add_subdirectory(core/configuration_handler)
add_subdirectory(core/request_response_handler)

add_subdirectory(core/handler_invoker)
add_subdirectory(clientserveriface)

add_subdirectory(client)
add_subdirectory(server)


install(TARGETS
        connection
        io_context_handler
        handler_invoker
        logger
        utils
        worker
        session
        message
        message_handler
        transport_handler
        configuration_handler
        clientserveriface
        request_response_handler
        segment_client
        segment_server
        client
        server

        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
)