#include <iostream>

#include <server.hpp>

int main()
{
	Server server("127.0.0.1", 8888);
	server.start();
}
