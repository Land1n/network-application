#pragma once

#include <cassert>
#include <iostream>
#include <WinSock2.h>

#pragma comment(lib, "Ws2_32.lib")


namespace Net
{
    class Server
    {
    private:
        WSADATA wsa;
        SOCKET server_socket;
        SOCKET client_socket;
        std::string ipaddress;
        int port;
        struct sockaddr_in server_info;
        int server_infolength;
        struct sockaddr_in client_info;
        int client_infolength;
        int recvlength;
        char buffer[1024];
    public:
        Server(int,std::string);
        ~Server();
    public:
        void start();
        void stop();
    private:
        void init();
        void receive();
        void proccess();
        void send();
    };
}