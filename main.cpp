#include <iostream>
#include "tcpServer.h"

void onConnection(const yeyj::TcpConnectionPtr & conn)
{
	std::cout << "new connection : " << conn->getId() << std::endl;
}

void onMessage(const yeyj::TcpConnectionPtr & conn)
{
	std::string data = conn->receiveRawAsString();
	conn->log(LOG_DEBUG, data);
	conn->sendRaw(data);
}

void onDisconnection(const yeyj::TcpConnectionPtr & conn)
{
	std::cout << "disconnection : " << conn->getId() << std::endl;
}

int main()
{
	yeyj::TcpServer server(44350);
	server.setConnectionCallback(onConnection);
	server.setMessageCallback(onMessage);
	server.setDisconnectionCallback(onDisconnection);
	server.start(2);
	return 0;
}
