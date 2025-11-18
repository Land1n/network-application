#include "server.h"


namespace Net 
{
    Server::Server(int port, std::string ipaddress)
        :
        wsa{ 0 },
        port(port),
        ipaddress(ipaddress),
        server_socket(INVALID_SOCKET),
        server_info{ 0 },
        server_infolength(sizeof(server_info)) {}
    void Server::init()
    {
        server_info.sin_family = AF_INET;
        server_info.sin_port = htons(port);
        server_info.sin_addr.s_addr = inet_addr(ipaddress.c_str());

        printf("WSA init: ");
        assert(!(WSAStartup(MAKEWORD(2,2),&wsa)) && "Error: WSA init");
        printf("success\n");


        printf("Socket creating: ");
        assert(((server_socket = socket(AF_INET, SOCK_STREAM, 0)) != SOCKET_ERROR) && "Error: Socket creating");
        printf("success\n");
        

        printf("Socket bind: ");
        assert((bind(server_socket, (struct sockaddr*)&server_info, server_infolength)) != SOCKET_ERROR && "Error: Socket bind");
        printf("success\n");
        
        
        printf("Socket listen: ");
        assert((listen(server_socket, SOMAXCONN)) != SOCKET_ERROR && "Error: Socket listen");
        printf("success\n");
        
        
        printf("Server start at %s:%d\n",inet_ntoa(server_info.sin_addr),ntohs(server_info.sin_port));
    }
    void Server::start()
    {
        init();
     
        for (;;)
        {    
            printf("Waiting for connection...\n");
            
            client_socket = accept(server_socket, (struct sockaddr*)&client_info, &client_infolength);
            if (client_socket == INVALID_SOCKET) {
                printf("accept() failed... %d\n", WSAGetLastError());
            }

            printf("Client connected: %s:%d\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));   


            while (true)
            {
                receive();
                if (recvlength <= 0) {
                    printf("Client disconnected or error occurred\n");
                    break;
                }
                proccess();
                send();
            }
        }
    }

    void Server::receive()
    {

        printf("Packet from: %s:%d\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));
        printf("Data: %s\n", buffer);

        if ((recvlength = recv(client_socket,buffer,1024,0)) == SOCKET_ERROR)
        {
            printf("recv() failed ... %d\n",WSAGetLastError());
            exit(EXIT_FAILURE);
        }
    }
    void Server::proccess()
    {
        printf("packet from:%s:%d\n",inet_ntoa(server_info.sin_addr),ntohs(server_info.sin_port));
        printf("Client response: %s\n", buffer);
    }
    void Server::send()
    {
        if ((::send(client_socket,buffer,recvlength,0)) == SOCKET_ERROR)
        {
            printf("send() failed... %d\n",WSAGetLastError());
            exit(EXIT_FAILURE);
        }
    }

    void Server::stop()
    {
        closesocket(server_socket);
        WSACleanup();
        printf("Server stopped\n");
    }
    Server::~Server()
    {
        stop();
    }
}