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

#include "sdrlogger/sdrlogger.h"


void Client::start() {
    auto& logger = BaseLogger::get();
    logger.init5Levels();
    ConnectionHandler connection_handler(address,port,io_context);
    auto socket = connection_handler.connect();
    logger("INFO") << "Client started" << "\n";

    logger("INFO") << "Write type message signal/information (1-2): " << "\n";
    int i = 0;
    std::cin >> i;
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
        transport_handler.send(transport_message);

        // Получения Response TransportMessage
        TransportMessage new_transport_message = transport_handler.read();
        // Парсим Response TransportMessage -> Response Message
        MessageHandler message_handler;
        if (new_transport_message.type == "signal") {
            auto new_message = message_handler.parse(new_transport_message);
            auto* new_message_signal = dynamic_cast<SignalMessage*>(new_message.get());

            // Выводим данные
            logger("INFO") << "Data from message: " << "\n";
            std::cout << new_message_signal->getSignal().at(0) << "\n";
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
        transport_handler.send(transport_message);

        // Получения Response TransportMessage
        TransportMessage new_transport_message = transport_handler.read();
        // Парсим Response TransportMessage -> Response Message
        MessageHandler message_handler;
        if (new_transport_message.type == "information") {
            auto new_message = message_handler.parse(new_transport_message);
            auto* new_message_signal = dynamic_cast<InformationMessage*>(new_message.get());

            // Выводим данные
            logger("INFO") << "Data from message: " << new_message_signal->getNumberCore() << "\n";
        }
    } else {
        logger("ERROR") << "Data from message: fail" << "\n";
        return;
    }

}

Client::Client(std::string address, int port) : address(address), port(port) {}
