//
// Created by ivan on 14.03.2026.
//
#include "Client.hpp"

#include <iostream>

#include "TransportHandler.hpp"
#include "ConnectionHandler.hpp"
#include "MessageHandler.hpp"

#include "SignalMessage.hpp"
#include "InformationMessage.hpp"

void Client::start() {

    std::cout << "Client started" << std::endl;
    std::cout << "1-2: " << std::endl;
    int i = 0;
    std::cin >> i;

    ConnectionHandler connection_handler(address,port,io_context);
    auto socket = connection_handler.connect();
    if (socket == nullptr) {
        std::cout << "Failed to connect to the server" << std::endl;
        return;
    } else {
        std::cout << "Successfully connected to the server" << std::endl;
    }
    if (i == 1){
        // Отправка Request TransportMessage
        TransportHandler transport_handler(socket);
        std::string json_str = R"({
            "type": "signal",
            "central_Freq": 0,
            "signal": []
        })";
        std::vector<uint8_t> payload(json_str.begin(), json_str.end());
        TransportMessage transport_message(payload);
        std::cout << "Send Signal TransportMessage" << std::endl;
        transport_handler.send(transport_message);

        // Получения Response TransportMessage
        std::cout << "Read new TransportMessage" << std::endl;
        TransportMessage new_transport_message = transport_handler.read();
        std::cout << "new_transport_message.type: " << new_transport_message.type << std::endl;
        // Парсим Response TransportMessage -> Response Message
        MessageHandler message_handler;
        if (new_transport_message.type == "signal") {
            auto new_message = message_handler.parse(new_transport_message);
            auto* new_message_signal = dynamic_cast<SignalMessage*>(new_message.get());

            // Выводим данные
            for (auto i : new_message_signal->getSignal())
                std::cout << i << " ";
        }
    } else if (i == 2) {
                // Отправка Request TransportMessage
        TransportHandler transport_handler(socket);
        std::string json_str = R"({
            "type": "information",
            "numberCore": 0
        })";
        std::vector<uint8_t> payload(json_str.begin(), json_str.end());
        TransportMessage transport_message(payload);
        std::cout << "Send Information TransportMessage" << std::endl;
        transport_handler.send(transport_message);

        // Получения Response TransportMessage
        std::cout << "Read new TransportMessage" << std::endl;
        TransportMessage new_transport_message = transport_handler.read();
        std::cout << "new_transport_message.type: " << new_transport_message.type << std::endl;
        // Парсим Response TransportMessage -> Response Message
        MessageHandler message_handler;
        if (new_transport_message.type == "information") {
            auto new_message = message_handler.parse(new_transport_message);
            auto* new_message_signal = dynamic_cast<InformationMessage*>(new_message.get());

            // Выводим данные
            std::cout << new_message_signal->getNumberCore() << std::endl;
        }
    } else {
        std::cout << "fail" << std::endl;
        return;
    }

}

Client::Client(std::string address, int port) : address(address), port(port) {}
