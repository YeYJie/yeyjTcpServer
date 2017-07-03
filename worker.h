#ifndef _WORKER_H_
#define _WORKER_H_

#include "include.h"
#include "thread.h"
#include "tcpConnection.h"
#include "mutex.h"
#include "hash.h"

namespace yeyj
{

class TcpServer;
class TcpConnection;

typedef std::shared_ptr<TcpConnection> 				TcpConnectionPtr;
typedef yeyj::HashTable<uint64_t, TcpConnectionPtr> ConnectionPool_t;

class Worker : public Thread
{
public:

	explicit Worker(TcpServer * master);

	~Worker();

	Worker(const Worker &) = delete;

	Worker & operator=(const Worker &) = delete;

	void setMaxConnection(const int n);

	void start();

	void stop();


	void registerNewConnection(const TcpConnectionPtr & conn);

	int getConnectionNum() const;


private:

	void workFunction();

	void registerNewConnection();

	void eviction();

	void manageInactiveConnection();

	void removeConnection(const TcpConnectionPtr & conn);

	void evictRandomN(int n, bool force);

private:

	int 							_epollfd;
	int 							_maxConnection;

	yeyj::MutexLock 				_mutex;
	std::queue<TcpConnectionPtr> 	_incomingConnection;

	ConnectionPool_t				_connectionPool;

	yeyj::TcpServer * 				_master = nullptr;
};

}

#endif
