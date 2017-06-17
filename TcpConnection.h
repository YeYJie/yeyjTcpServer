#ifndef _TCPCONNECTION_H_
#define _TCPCONNECTION_H_

#include "include.h"
#include "InetSockAddr.h"
#include "buffer.h"

namespace yeyj
{

class TcpConnection
{
public:

typedef std::function<void (TcpConnection & conn)> 	ConnectionCallback;
typedef std::function<void (TcpConnection & conn)> 	DisconnectionCallback;
typedef std::function<void (TcpConnection & conn)> 	MessageCallback;

	explicit TcpConnection(const int & connfd,
						   const InetSockAddr & peerAddr,
						   ConnectionCallback connectionCallback,
						   DisconnectionCallback disconnectionCallback,
						   MessageCallback messageCallback,
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

	ConnectionCallback 			_connectionCallback;
	DisconnectionCallback 		_disconnectionCallback;
	MessageCallback 			_messageCallback;

	Buffer						_readBuffer;
	Buffer 						_writeBuffer;

	int 						_lastActiveTime;
};

typedef std::function<void (TcpConnection & conn)> 	ConnectionCallback;
typedef std::function<void (TcpConnection & conn)> 	DisconnectionCallback;
typedef std::function<void (TcpConnection & conn)> 	MessageCallback;

}

#endif
