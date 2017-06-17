#ifndef _TCPCONNECTION_H_
#define _TCPCONNECTION_H_

#include "include.h"
#include "InetSockAddr.h"
#include "buffer.h"
#include "TcpServer.h"

namespace yeyj
{

class TcpServer;

class TcpConnection
{
public:

	explicit TcpConnection(const int & connfd,
						   const InetSockAddr & peerAddr,
						   TcpServer * master,
						   int read_buffer_init_size,
						   int read_buffer_max_size,
						   int write_buffer_init_size,
						   int write_buffer_max_size);
	~TcpConnection();


	void send(const std::string & str);
	void send(const char * str);
	void send(const char * str, int size);

	std::string receiveAsString();
	std::string receiveAsString(int length);
	int receive(char * dst);
	int receive(char * dst, int length);


	int getfd();

	epoll_event * getEpollEvent();

	void onConnection();
	void onDisconnection();

	// void onReadableEvent();
	void onReadableEvent();
	void onWritableEvent();

	void onMessage();

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

	TcpServer * 				_master;

	Buffer						_readBuffer;
	Buffer 						_writeBuffer;

	int 						_lastActiveTime;
};



}

#endif
