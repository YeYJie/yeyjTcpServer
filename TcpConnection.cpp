#include "TcpConnection.h"
#include <strings.h>
#include <unistd.h>
#include <cassert>
#include <fcntl.h>
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

#include <cstring>
void TcpConnection::handleRead()
{
	char buffer[200];
	int len;
	bzero(buffer, sizeof(buffer));
	// read(_connfd, buffer, sizeof(buffer));
	if((len = read(_connfd, buffer, sizeof(buffer)) == 0)) {
		printf("TcpConnection::handleRead close\n");
		assert(close(_connfd) == 0);
	}
	
	char ok[20];
	sprintf(ok, "HTTP/1.1 200 OK\r\n");
	write(_connfd, ok, sizeof(ok));

	// printf("TcpConnection::handleRead [%d] [%d]%s\n",
	//         _connfd, strlen(buffer), buffer);
	printf("TcpConnection::handleRead [%d] [%ld]",
			_connfd, strlen(buffer));
	for(int i = 0; i < strlen(buffer); ++i)
		printf("%d ", buffer[i]);
	printf("\n");
}
