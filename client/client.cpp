#include "client.h"

namespace Net 
{
    Client::Client(int port, std::string ipaddress)
        :
        wsa{ 0 },
        port(port),
        ipaddress(ipaddress),
        client_socket(INVALID_SOCKET),
        server_info{ 0 },
        server_infolength(sizeof(server_info)),
        recvlength(0)
    {
        buffer[0] = '\0';
    }

    void Client::init()
    {
        server_info.sin_family = AF_INET;
        server_info.sin_port = htons(port);
        server_info.sin_addr.s_addr = inet_addr(ipaddress.c_str());

        printf("WSA init: ");
        assert(!(WSAStartup(MAKEWORD(2,2),&wsa)) && "Error: WSA init");
        printf("success\n");

        printf("Socket creating: ");
        assert(((client_socket = socket(AF_INET, SOCK_STREAM, 0)) != INVALID_SOCKET) && "Error: Socket creating");
        printf("success\n");
    }

    void Client::connect()
    {
        init();
        
        printf("Connecting to server: ");
        if (::connect(client_socket, (struct sockaddr*)&server_info, server_infolength) == SOCKET_ERROR)
        {
            printf("Failed to connect to server: %d\n", WSAGetLastError());
            closesocket(client_socket);
            WSACleanup();
            return;
        }
        printf("success\n");
        printf("Connected to server: %s:%d\n", ipaddress.c_str(), port);
        
        for (;;)
        {
            send();
            receive();
            proccess();
        }
    }

    void Client::receive()
    {
        if ((recvlength = recv(client_socket, buffer, sizeof(buffer)-1, 0)) == SOCKET_ERROR)
        {
            printf("recv() failed ... %d\n", WSAGetLastError());
            return;
        }
    }

    void Client::proccess()
    {
        printf("Server response: %s\n", buffer);
    }

    void Client::send()
    {
        printf("Enter message: ");
        std::string message;
        std::getline(std::cin, message); 
        strncpy(buffer, message.c_str(), sizeof(buffer));
        
        if ((::send(client_socket, buffer, message.length(), 0)) == SOCKET_ERROR)
        {
            printf("send() failed... %d\n", WSAGetLastError());
            return;
        }
        printf("Message sent: %s\n", message.c_str());
    }

    Client::~Client()
    {
        closesocket(client_socket);
        WSACleanup();
    }
}