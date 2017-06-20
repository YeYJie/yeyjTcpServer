#include "tcpServer.h"
using namespace yeyj;
using namespace std;

// int totalConnection = 0;

void onConnection(const TcpConnectionPtr & conn)
{
	// totalConnection++;
	cout << "new connection : " << conn->getId() << endl;
}

void onMessage(const TcpConnectionPtr & conn)
{
	string message = conn->receiveAsString();
	conn->send(message);
	cout << message << endl;
}

void onDisconnection(const TcpConnectionPtr & conn)
{
	// totalConnection--;
	cout << "disconnection : " << conn->getId() << endl;
}

int main()
{
	TcpServer server(44350);
	server.setConnectionCallback(onConnection);
	server.setMessageCallback(onMessage);
	server.setDisconnectionCallback(onDisconnection);
	server.start(1);
	return 0;
}
