#ifndef _TCPCONNECTION_H_
#define _TCPCONNECTION_H_

#include "include.h"
#include "inetSockAddr.h"
#include "buffer.h"
#include "tcpServer.h"
#include "worker.h"

namespace yeyj
{

class TcpServer;
class Worker;

class TcpConnection : public enable_shared_from_this<TcpConnection>
{
public:

	explicit TcpConnection(uint64_t id,
						   const int & connfd,
						   const InetSockAddr & peerAddr,
						   TcpServer * master,
						   Worker * worker,
						   int read_buffer_init_size,
						   int read_buffer_max_size,
						   int write_buffer_init_size,
						   int write_buffer_max_size);
	~TcpConnection();


	void sendRaw(const std::string & str);
	void sendRaw(const char * str);
	void sendRaw(const char * str, int size);

	void sendMessage(const std::string & str);
	void sendMessage(const char * str);
	void sendMessage(const char * str, int size);

	std::string receiveRawAsString();
	std::string receiveRawAsString(int length);
	int receiveRaw(char * dst);
	int receiveRaw(char * dst, int length);

	std::string receiveMessageAsString();
	int receiveMessage(char * dst);


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

	void log(const char * level, const string & msg);
	// {
	// 	_master->log(level, _worker->getTid(), _worker->getName(),
	// 				_peerAddr.getIP(), _peerAddr.getPort(), msg);
	// }

private:

	uint64_t 					_id;

	int 						_connfd;
	struct epoll_event 			_epollEvent;

	InetSockAddr 				_localAdddr;
	InetSockAddr 				_peerAddr;

	TcpServer * 				_master;
	Worker * 					_worker;

	Buffer						_readBuffer;
	Buffer 						_writeBuffer;

	int 						_lastActiveTime;
	bool 						_close;
};

typedef shared_ptr<TcpConnection> TcpConnectionPtr;

}

#endif
