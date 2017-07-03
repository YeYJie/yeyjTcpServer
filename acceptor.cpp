#include "acceptor.h"
#include "asyncLogging.h"
using namespace yeyj;

Acceptor::Acceptor(const int & port) :
	_port(port)
{
	_listenfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(_listenfd >= 0);

	int shit = 1;
	setsockopt(_listenfd, SOL_SOCKET, SO_REUSEADDR,
			   &shit, sizeof(int));

	int flags = fcntl(_listenfd, F_GETFL, 0);
	assert(flags >= 0);
	assert(fcntl(_listenfd, F_SETFL, flags | O_NONBLOCK) >= 0);

	InetSockAddr serverAddr(AF_INET, INADDR_ANY, _port);

	if((bind(_listenfd,
			 (struct sockaddr *)&serverAddr,
			 sizeof(serverAddr))) != 0)
		printf("\nAcceptor::Acceptor [%s]\n\n", strerror(errno));
	assert(listen(_listenfd, SOMAXCONN) == 0);


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

void Acceptor::setServerCron(const ServerCron & sc)
{
	_serverCron = sc;
}

Acceptor::~Acceptor()
{

}

void Acceptor::start(int timeout)
{
	std::cout << "acceptor start..." << std::endl;
	/* should only be ran in the TcpServer main thread */
	/* epoll wait and accept */
	struct epoll_event events[1];
	while(true) {
		int nfds = epoll_wait(_epollfd, events, 1, timeout);

		// assert(nfds == 1);
		// assert(events[0].data.fd == _listenfd);

		if(nfds == 1)
		{
			int ret = 0;
			while(true) { // edge-trigger
				sockaddr_in clientAddr;
				unsigned int clientAddrLen = sizeof(clientAddr);
				int ret = accept(_listenfd,
									(struct sockaddr *)&clientAddr,
									&clientAddrLen);
				if(ret < 0) {
					if(errno == EAGAIN || errno == EWOULDBLOCK
						|| errno == ECONNABORTED || errno == EPROTO
						|| errno == EINTR)
						break;
					else
						std::cout << "Acceptor::start ret < 0" << std::endl;
				}
				assert(ret >= 0);
				_connectionCallback(ret, InetSockAddr(clientAddr));
			}
		}

		_serverCron();
	}
}
