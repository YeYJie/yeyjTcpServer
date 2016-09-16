#include "Acceptor.h"
#include "AsyncLogging.h"
#include <strings.h> /* bzero */
#include <sys/socket.h>
#include <cassert>
using namespace yeyj;

Acceptor::Acceptor(const int & port) :
	_port(port),
	_running(false)
{
	/* create a listen socket on the given port */
	_listenfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(_listenfd >= 0);	

	InetSockAddr	serverAddr;
	bzero((void *)serverAddr, sizeof(serverAddr));	
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons((unsigned short)_port);
	
	assert(bind(_listenfd, 
				(struct sockaddr *)&serverAddr, 
				sizeof(serverAddr)) == 0);
	assert(listen(_listenfd, SOMAXCONN) == 0);


	/* create epoll fd */
	_epollfd = epoll_create(1);
	assert(_epollfd >= 0);
	
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.u64 = 0LL; /* what the shit ? */
	ev.data.fd = _listenfd;

	epoll_ctl(_epollfd, EPOLL_CTL_ADD, _listenfd, &ev);
}

void setConnectionCallback(const ConnectionCallback & cb)
{
	_connectionCallback = cb;
}

Acceptor::~Acceptor()
{

}

void Acceptor::start()
{
	/* should only be ran in the TcpServer main thread */ 
	/* epoll wait and accept */
	struct epoll_event events[1];
	while(true) {
		int nfds = epoll_wait(_epollfd, events, 1, -1);	
		
		/* once epoll_wait return, nfds should be 1 */
		assert(nfds == 1);
		assert(events[0].data.fd == _listenfd);

		InetSockAddr clientAddr;		
		int connectSock = accept(_listenfd, 
								 (struct sockaddr *)&clientAddr,
								 sizeof(clientAddr));
		assert(connectSock >= 0); /* assert or log ? */
		
		/* here we already get client addr info : */
		/* 		connectSock and clientAddr 		  */
		_connectionCallback(connectSock, clientAddr);
	}
}
