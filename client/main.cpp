//
// Created by ivan on 14.03.2026.
//

#include "Client.hpp"

int main() {

    Client client("127.0.0.1",8080);
    client.start();

    return 0;
}
