#include <iostream>
#include <WinSock2.h>

#pragma comment(lib, "Ws2_32.lib")

int main()
{
    WSAData wsaData;
    WORD DLLVersion = MAKEWORD(2, 1);
    if (WSAStartup(DLLVersion, &wsaData) != 0)
    {
        std::cout << "Error: WSAStartup" << std::endl;
        return 1;
    }

    SOCKADDR_IN addr;
    int sizeofaddr = sizeof(addr);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(1000); 
    addr.sin_family = AF_INET;

    SOCKET Connection = socket(AF_INET, SOCK_STREAM, 0);
    if (Connection == INVALID_SOCKET)
    {
        std::cout << "Error: Failed to create socket" << std::endl;
        WSACleanup();
        return 1;
    }

    if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0)
    {
        std::cout << "Error: Failed to connect to server" << std::endl;
        closesocket(Connection);
        WSACleanup();
        return 1;
    }
    
    std::cout << "Connected to server" << std::endl;

 

    system("pause");
    return 0;
}