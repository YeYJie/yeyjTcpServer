#ifndef TCPCONNECTION_H_
#define TCPCONNECTION_H_

#include "InetSockAddr.h"
#include <sys/epoll.h>
#include <functional>

namespace yeyj
{

class TcpConnection
{
public:

	typedef std::function<void ()> 	ConnectionCallback;
	typedef std::function<void ()> 	MessageCallback;

	explicit TcpConnection(const int & connfd,
						   const InetSockAddr & peerAddr,
						   ConnectionCallback connectionCallback,
						   MessageCallback messageCallback);
	~TcpConnection();

	void send();

	void receive();

	int getfd();

	epoll_event * getEpollEvent();

	void handleRead();

private:
	
	int 						_connfd;
	struct epoll_event 			_epollEvent;

	InetSockAddr 				_localAdddr;
	InetSockAddr 				_peerAddr;
	
	ConnectionCallback 			_connectionCallback;
	MessageCallback 			_messageCallback;

	// Buffer					_readBuffer;
	// Buffer 					_writeBuffer;
};

}

#endif
