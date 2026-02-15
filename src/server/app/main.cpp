#include <iostream>
#include "connection_handler.hpp"


int main() {
    ConnectionHandler handler("127.0.0.1", 8000);
    handler.start();
    return 0;
}