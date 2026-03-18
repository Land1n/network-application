//
// Created by ivan on 14.03.2026.
//
#include "Client.hpp"

#include <iostream>

#include "TransportHandler.hpp"
#include "ConnectionHandler.hpp"
#include "MessageHandler.hpp"

#include "SignalMessage.hpp"


void Client::start() {
    ConnectionHandler connection_handler(address,port,io_context);
    auto socket = connection_handler.connect();


    // Отправка Request TransportMessage
    TransportHandler transport_handler(socket);
    std::string type = "signal";
    TransportMessage message( type,{});
    transport_handler.send(message);


    // Получения Response TransportMessage
    TransportMessage new_transport_message = transport_handler.read();

    // Парсим Response TransportMessage -> Response Message
    MessageHandler message_handler;
    auto new_message = message_handler.parse(new_transport_message);
    auto* new_message_signal = dynamic_cast<SignalMessage*>(new_message.get());

    // Выводим данные
    for (auto i : new_message_signal->getSignal())
        std::cout << i << " ";
}

Client::Client(std::string address, int port) : address(address), port(port) {}
