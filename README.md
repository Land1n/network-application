# Chat System (Client-Server)

Простой клиент-серверный чат-система, использующая Boost.Asio для сетевого взаимодействия и Boost.JSON для сериализации данных.

## Структура проекта
> project/
> ├── client/ # Клиентская часть
> │ └── client.cpp # Основной файл клиента
> ├── server/ # Серверная часть
> │ ├── server.hpp/cpp # Основной класс сервера
> │ ├── server_session.hpp/cpp # Обработка клиентских сессий
> │ └── main.cpp # Точка входа сервера
> ├── core/ # Общая логика
> │ ├── include/
> │ │ └── request_response_handler.hpp
> │ └── request_response_handler.cpp # Обработка JSON сообщений
> ├── tests/ # Тесты
> │ ├── test_server.cpp
> │ └── test_request_response_handler.cpp
> └── CMakeLists.txt # Корневой CMake файл

## Основные файлы

### Клиент (`client/client.cpp`)
- Подключается к серверу по указанному IP-адресу и порту
- Отправляет команды на сервер и выводит ответы
- Использует `RequestResponseHandler` для сериализации/десериализации

### Сервер (`server/`)
- **`server.hpp/cpp`** - запуск сервера, управление подключениями
- **`server_session.hpp/cpp`** - обработка отдельных клиентских сессий, чтение/запись данных
- **`main.cpp`** - точка входа сервера

### Обработчик сообщений (`core/`)
- **`MessageData`** - структура для хранения команды и данных
- **`RequestResponseHandler`** - парсинг и сериализация JSON, обработка команд

### Тесты (`tests/`)
- Юнит-тесты для `RequestResponseHandler` и `Server`

## Поддерживаемые команды сервера

Сервер обрабатывает следующие команды, отправляемые в поле `command` JSON-сообщения:

1. **`ping`** - тестовая команда, сервер отвечает `pong` с увеличенным счетчиком
   ```json
   {"command": "ping", "data": [5]} → {"command": "pong", "data": [6]}
getData - запрос данных, сервер отвечает sendData с нулевыми данными

json
{"command": "getData", "data": [100]} → {"command": "sendData", "data": [0]}
Любая другая команда возвращает ошибку:

json
{"command": "unknown", "data": [...]} → {"command": "error", "data": [-1]}
