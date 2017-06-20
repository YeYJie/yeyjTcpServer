#ifndef _TCPCONNECTION_H_
#define _TCPCONNECTION_H_

#include "include.h"
#include "InetSockAddr.h"
#include "buffer.h"
#include "TcpServer.h"

namespace yeyj
{

class TcpServer;

class TcpConnection : public enable_shared_from_this<TcpConnection>
{
public:

	explicit TcpConnection(uint64_t id,
						   const int & connfd,
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


	int getfd() const { return _connfd; }

	epoll_event * getEpollEvent() { return &_epollEvent; }

	void onConnection();
	void onDisconnection();

	// void onReadableEvent();
	void onReadableEvent();
	void onWritableEvent();

	void onMessage();

	uint64_t getId() const { return _id; }

	int getLastActiveTime() const { return _lastActiveTime; }

	bool close() const { return _close; }

	void updateEpollEvent() {
		if(_close) return;
		if(!_writeBuffer.empty())
			_epollEvent.events |= EPOLLOUT;
		else
			_epollEvent.events &= ~EPOLLOUT;
	}

private:

	uint64_t 					_id;

	int 						_connfd;
	struct epoll_event 			_epollEvent;

	InetSockAddr 				_localAdddr;
	InetSockAddr 				_peerAddr;

	TcpServer * 				_master;

	Buffer						_readBuffer;
	Buffer 						_writeBuffer;

	int 						_lastActiveTime;
	bool 						_close;
};

typedef shared_ptr<TcpConnection> TcpConnectionPtr;

}

#endif
