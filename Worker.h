#ifndef _WORKER_H_
#define _WORKER_H_

#include "include.h"
// #include "InetSockAddr.h"
#include "Thread.h"
#include "TcpConnection.h"
#include "Mutex.h"
// #include "TcpServer.h"

namespace yeyj
{

class TcpServer;
class TcpConnection;
typedef shared_ptr<TcpConnection> TcpConnectionPtr;

class Worker : public Thread
{
public:

	// explicit Worker(const int & maxConnection);
	explicit Worker(TcpServer * master);

	void setMaxConnection(const int n) {
		_maxConnection = n;
	}

	void start();

	void stop();

	/*
	 * 	This function would be called by the TcpServer thread
	 * */
	void registerNewConnection(const TcpConnectionPtr & conn);

	int getConnectionNum() const { return _connectionPool.size(); }

private:

	/*
	 * 	use epoll_wait
	 *	when the conn socket becomes readable, calls the responding
	 *	TcpConnection's handle read callback function
	 * */
	void workFunction();

	void registerNewConnection();

	void eviction();

	void manageInactiveConnection();

	int 							_epollfd;
	int 							_maxConnection;

	MutexLock 						_mutex;
	std::queue<TcpConnectionPtr> 	_incomingConnection;

	// std::unordered_map<int, TcpConnectionPtr> 	_connectionPool;
	std::vector<TcpConnectionPtr>	_connectionPool;

	TcpServer * 					_master;
};

}

#endif
