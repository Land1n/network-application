#pragma once

#include <cassert>
#include <iostream>
#include <WinSock2.h>

#pragma comment(lib, "Ws2_32.lib")


namespace Net
{
    class Client
    {
    private:
        WSADATA wsa;
        SOCKET client_socket;
        std::string ipaddress;
        int port;
        struct sockaddr_in server_info;
        int server_infolength;
        int recvlength;
        char buffer[1024];
    public:
        Client(int,std::string);
        ~Client();
    public:
        void connect();
    private:
        void init();
        void send();
        void receive();
        void proccess();
    };
}