#ifndef _TCPSERVER_H_
#define _TCPSERVER_H_

#include "include.h"
#include "Acceptor.h"
#include "AsyncLogging.h"
#include "InetSockAddr.h"
#include "TcpConnection.h"
#include "Worker.h"

namespace yeyj
{

#define MAX_TCP_EVICTION_NONE 			0
#define MAX_TCP_EVICTION_RANDOM 		1
#define MAX_TCP_EVICTION_INACTIVE 		2

#define INACTIVE_TCP_EVICTION_NONE 		0
#define INACTIVE_TCP_EVICTION_CLOSE 	1

#define LOAD_BALANCE_ROUNDROBIN 		0
#define LOAD_BALANCE_RANDOM 			1
#define LOAD_BALANCE_MINCONNECTION 		2

class Worker;

class TcpServer
{
public:

	explicit TcpServer(int port);

	~TcpServer();

	typedef std::function<Worker*()> LoadBalanceFunc;

	/*
	 *	runs the tcpserver with 'n' worker threads
	 * */
	void start(const int & n);

	void stop();

	void setConnectionCallback(const ConnectionCallback & cb);

	void setDisconnectionCallback(const DisconnectionCallback & cb);

	void setMessageCallback(const MessageCallback & cb);

private:

	/*
	 *  once a new connection is accept, this function will be called
	 *  it finds a thread from the threadPool to work on that connection
	 * */
	void newConnection(int connSock, InetSockAddr peerAddr);

	// Worker * findAWorker();

	void loadConfig(const char * configFileName);

private:

	std::string 						_name;

	Acceptor 							_acceptor;

	std::vector<Worker *> 		_threadPool;

	ConnectionCallback 			_connectionCallback;
	DisconnectionCallback 		_disconnectionCallback;
	MessageCallback 			_messageCallback;

	LoadBalanceFunc				_loadBalance;

	int _listenning_port;

	int _max_tcp;

	int _max_tcp_per_worker;

	int _max_tcp_eviction_rule;

	int _inactive_tcp_timeout;

	int _inactive_tcp_eviction_rule;

	// int _load_balance;

	int _tcp_read_buffer_init_size_bytes;
	int _tcp_read_buffer_max_size_bytes;

	int _tcp_write_buffer_init_size_bytes;
	int _tcp_write_buffer_max_size_bytes;

	string _log_file_name_prefix;

	int _log_file_max_size;

	int _log_flush_interval_second;

	int _log_high_watermask;



public: // getter

	int getListenningPort() const;
	int getMaxTcpConnection() const;
	int getMaxTcpConnectionPerWorker() const;
	int getMaxTcpEvictionRule() const;
	int getInactiveTcpTimeOut() const;
	int getInactiveTcpEvictionRule() const;
	int getReadBufferInitSize() const;
	int getReadBufferMaxSize() const;
	int getWriteBufferInitSize() const;
	int getWriteBufferMaxSize() const;
	string getLogFileNamePrefix() const;
	int getLogFileNameSize() const;
	int getLogFlushInterval() const;
	int getLogHighWaterMask() const;

private: // load balance functions

	Worker * loadBalanceRoundRobin();
	Worker * loadBalanceRandom();
	Worker * loadBalanceMinConnection();
};

}

#endif
