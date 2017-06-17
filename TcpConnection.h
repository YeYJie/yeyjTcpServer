#ifndef _TCPCONNECTION_H_
#define _TCPCONNECTION_H_

#include "include.h"
#include "InetSockAddr.h"


namespace yeyj
{

class TcpConnection
{
public:

	typedef std::function<void ()> 	ConnectionCallback;
	typedef std::function<void ()> 	DisconnectionCallback;
	typedef std::function<void ()> 	MessageCallback;

	explicit TcpConnection(const int & connfd,
						   const InetSockAddr & peerAddr,
						   ConnectionCallback connectionCallback,
						   // DisconnectionCallback disconnectionCallback,
						   MessageCallback messageCallback);
	~TcpConnection();

	void send();

	void receive();

	int getfd();

	epoll_event * getEpollEvent();

	void onConnection();
	void onDisconnection();

	void onData();

	void onMessage(char * buffer);

	int getHash()
	{
		// temp hash function
		return _localAdddr.getIP() | _localAdddr.getPort() | _peerAddr.getIP() | _peerAddr.getPort();
	}

private:

	int 						_connfd;
	struct epoll_event 			_epollEvent;

	InetSockAddr 				_localAdddr;
	InetSockAddr 				_peerAddr;

	ConnectionCallback 			_connectionCallback;
	DisconnectionCallback 		_disconnectionCallback;
	MessageCallback 			_messageCallback;

	// Buffer					_readBuffer;
	// Buffer 					_writeBuffer;
};

}

#endif
