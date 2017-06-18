#include "Acceptor.h"
#include "AsyncLogging.h"
using namespace yeyj;

Acceptor::Acceptor(const int & port) :
	_port(port)
{
	/* create a listen socket on the given port */
	_listenfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(_listenfd >= 0);

	int shit = 1;
	setsockopt(_listenfd, SOL_SOCKET, SO_REUSEADDR,
			   &shit, sizeof(int));

	InetSockAddr serverAddr(AF_INET, INADDR_ANY, _port);

	// assert(bind(_listenfd,
	// 			(struct sockaddr *)&serverAddr,
	// 			sizeof(serverAddr)) == 0);
	if((bind(_listenfd,
			 (struct sockaddr *)&serverAddr,
			 sizeof(serverAddr))) != 0)
		printf("\nAcceptor::Acceptor [%s]\n\n", strerror(errno));
	assert(listen(_listenfd, SOMAXCONN) == 0);


	/* create epoll fd */
	_epollfd = epoll_create(1);
	assert(_epollfd >= 0);

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = _listenfd;

	epoll_ctl(_epollfd, EPOLL_CTL_ADD, _listenfd, &ev);
}

void Acceptor::setConnectionCallback(const ConnectionCallback & cb)
{
	_connectionCallback = cb;
}

Acceptor::~Acceptor()
{

}

void Acceptor::start()
{
	cout << "acceptor start..." << endl;
	/* should only be ran in the TcpServer main thread */
	/* epoll wait and accept */
	struct epoll_event events[1];
	while(true) {
		int nfds = epoll_wait(_epollfd, events, 1, -1);

		/* once epoll_wait return, nfds should be 1 */
		assert(nfds == 1);
		assert(events[0].data.fd == _listenfd);

		sockaddr_in clientAddr;
		unsigned int clientAddrLen = sizeof(clientAddr);
		int connectSock = accept(_listenfd,
								 (struct sockaddr *)&clientAddr,
								 &clientAddrLen);
		assert(connectSock >= 0); /* assert or log ? */

		// cout << "accept" << connectSock << endl;

		/* here we already get client addr info : */
		/* 		connectSock and clientAddr 		  */

		// printf("Acceptor::start\n");
		_connectionCallback(connectSock, InetSockAddr(clientAddr));
	}
}
