#include <iostream>
#include <WinSock2.h>

#pragma comment(lib, "Ws2_32.lib")

int main()
{
    WSAData wsaData;
    WORD DLLVersion = MAKEWORD(2,1);
    if(WSAStartup(DLLVersion,&wsaData) != 0)
    {
        std::cout << "Error:WSAStartup" << std::endl;
    }

    SOCKADDR_IN addr;
    int sizeofaddr = sizeof(addr);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(1000);
    addr.sin_family = AF_INET;

    SOCKET sListen = socket(AF_INET,SOCK_STREAM,0);
    bind(sListen, (SOCKADDR*)&addr,sizeof(addr));

    listen(sListen,SOMAXCONN);
    
    SOCKET newConnection;
    newConnection = accept(sListen,(SOCKADDR*)&addr,&sizeofaddr);
    
    if (newConnection == INVALID_SOCKET)
        std::cout << "Error:Connection" << std::endl;
    else
        std::cout << "Client connected" << std::endl;


    system("pause");

    return 0;
}