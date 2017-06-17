#include "TcpServer.h"
using namespace yeyj;
using namespace std;

int totalConnection = 0;

void onConnection(TcpConnection & conn)
{
	totalConnection++;
	cout << "new connection : " << totalConnection << endl;
}

void onMessage(TcpConnection & conn)
{
	string message = conn.receiveAsString();
	conn.send(message);
	cout << message << endl;
}

void onDisconnection(TcpConnection & conn)
{
	totalConnection--;
	cout << "disconnection : " << totalConnection << endl;
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
