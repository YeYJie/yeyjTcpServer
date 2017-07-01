#include "tcpServer.h"
using namespace yeyj;
using namespace std;

// int totalConnection = 0;

void onConnection(const TcpConnectionPtr & conn)
{
	// totalConnection++;
	cout << "new connection : " << conn->getId() << endl;
	conn->log(LOG_DEBUG, "yeyongjie");
}

void onMessage(const TcpConnectionPtr & conn)
{
	conn->sendRaw(conn->receiveRawAsString());

	// string message = conn->receiveRawAsString();
	// // conn->send(message);
	// cout << message << endl;
	// string reply = "HTTP/1.1 200 OK\r\n"
	// 			"Connection: close\r\n"
	// 			"Server: Apache/2.2.3 (CentOS)\r\n"
	// 			"Content-Length: 15\r\n"
	// 			"Content-Type: text/html\r\n"
	// 			"\r\n"
	// 			"<html></html>\r\n";
	// // cout << reply << endl;
	// cout << reply.size() << endl;
	// conn->sendRaw(reply);
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
	server.start(2);
	return 0;
}
