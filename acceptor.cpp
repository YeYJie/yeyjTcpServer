#include "acceptor.h"
#include "asyncLogging.h"
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
	ev.events = EPOLLIN | EPOLLET;
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

		int ret = 0;
		while(true) { // edge-trigger
			sockaddr_in clientAddr;
			unsigned int clientAddrLen = sizeof(clientAddr);
			int ret = accept(_listenfd,
								(struct sockaddr *)&clientAddr,
								&clientAddrLen);
			if(ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
				break;
			assert(ret >= 0);
			_connectionCallback(ret, InetSockAddr(clientAddr));
		}
	}
}
