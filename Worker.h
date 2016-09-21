#ifndef WORKER_H_
#define WORKER_H_

#include "InetSockAddr.h"
#include "Thread.h"
#include "TcpConnection.h"
#include "Mutex.h"
#include <queue>

namespace yeyj
{

class Worker : public Thread
{
public:

	explicit Worker(const int & maxConnection);
	
	void start();

	void stop();

	/*
	 * 	This function would be called by the TcpServer thread
	 * */
	void work(TcpConnection * conn);

private:
	
	/*
	 * 	use epoll_wait
	 *	when the conn socket becomes readable, calls the responding
	 *	TcpConnection's handle read callback function
	 * */
	void workFunction();

	void registerNewConnection();

	int 							_epollfd;
	int 							_maxConnection;

	MutexLock 						_mutex;
	std::queue<TcpConnection *> 	_incomingConnection;
};

}

#endif
