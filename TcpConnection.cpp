#include "TcpConnection.h"
using namespace yeyj;

TcpConnection::TcpConnection(const int & connfd,
							 const InetSockAddr & peerAddr,
							 ConnectionCallback connectionCallback,
							 MessageCallback messageCallback) :
	_connfd(connfd),
	_peerAddr(peerAddr),
	_connectionCallback(connectionCallback),
	_messageCallback(messageCallback)
{
	/*	set the connection socket as non-blocking socket
	 * */
	int flags = fcntl(_connfd, F_GETFL, 0);
	assert(flags >= 0);
	assert(fcntl(_connfd, F_SETFL, flags | O_NONBLOCK) >= 0);

	int shit = 1;
	setsockopt(_connfd, SOL_SOCKET, SO_KEEPALIVE,
			   &shit, sizeof(int));

	_epollEvent.events = 0;
	_epollEvent.events |= EPOLLIN;
	_epollEvent.data.ptr = this;
	// printf("TcpConnection::constructor\n");
}

TcpConnection::~TcpConnection()
{

}

void TcpConnection::send()
{

}

void TcpConnection::receive()
{
}

int TcpConnection::getfd()
{
	return _connfd;
}

epoll_event * TcpConnection::getEpollEvent()
{
	return &_epollEvent;
}

void TcpConnection::onConnection()
{
	cout << "new tcp connection" << endl;
	// _connectionCallback();
}

void TcpConnection::onData()
{
	// call by worker
	// read the comming data into readbuffer
	char buffer[200];
	int len = -1;
	bzero(buffer, sizeof(buffer));
	len = read(_connfd, buffer, sizeof(buffer));

	cout << "ondata [" << len << "]" << endl;

	if(len <= 0)
		onDisconnection();
	else
		onMessage(buffer);
}

void TcpConnection::onDisconnection()
{
	// _disconnectionCallback();
	cout << "tcp connection close" << endl;
	assert(close(_connfd) == 0);
}

void TcpConnection::onMessage(char * buffer)
{
	// _messageCallback();

	char ok[20];
	sprintf(ok, "HTTP/1.1 200 OK\r\n");
	write(_connfd, ok, sizeof(ok));

	printf("TcpConnection::handleRead [%d] [%ld]",
			_connfd, strlen(buffer));
	printf("[");
	for(int i = 0; i < strlen(buffer); ++i)
		printf("%c", buffer[i]);
	printf("]");
	printf("\n");
}
