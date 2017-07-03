#ifndef _TCPSERVER_H_
#define _TCPSERVER_H_

#include "include.h"
#include "acceptor.h"
#include "asyncLogging.h"
#include "worker.h"

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
class TcpConnection;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

typedef std::function<void (const TcpConnectionPtr & conn)> ConnectionCallback;
typedef std::function<void (const TcpConnectionPtr & conn)> DisconnectionCallback;
typedef std::function<void (const TcpConnectionPtr & conn)> MessageCallback;

typedef std::function<Worker*(uint32_t)> LoadBalanceFunc;



class TcpServer
{
public:

	explicit TcpServer(int port);

	~TcpServer();

	TcpServer(const TcpServer &) = delete;
	TcpServer & operator=(const TcpServer &) = delete;

	void start(const int & n);

	void stop();


	void setConnectionCallback(const ConnectionCallback & cb);

	void setDisconnectionCallback(const DisconnectionCallback & cb);

	void setMessageCallback(const MessageCallback & cb);


	ConnectionCallback 			connectionCallback;
	DisconnectionCallback 		disconnectionCallback;
	MessageCallback 			messageCallback;


	void log(const char * level, pthread_t threadID,
				const std::string & threadName,
				uint32_t ip, uint16_t port,
				const std::string & msg);

	bool exceedMaxMemory() const;

private:

	/*
	 *  once a new connection is accept, this function will be called
	 *  it finds a thread from the threadPool to work on that connection
	 * */
	void newConnection(int connSock, InetSockAddr peerAddr);

	void serverCron();

	void updateTime();

	void checkMaxMemory();

	void loadConfig(const char * configFileName);

private:

	bool 						_exceedMaxMemory = false;

	std::string 				_timeString;

	AsyncLogging 				_logger;

	std::string 				_name;
	pthread_t 					_tid = 0;

	Acceptor 					_acceptor;

	std::vector<Worker *> 		_threadPool;


	LoadBalanceFunc				_loadBalance;

	int 						_listenning_port;

	int 						_max_tcp;
	int 						_max_tcp_per_worker;
	int 						_max_tcp_eviction_rule;

	int 						_inactive_tcp_timeout;
	int 						_inactive_tcp_eviction_rule;

	int 						_max_vm_kb;
	int 						_max_rss_kb;

	int 						_eviction_pool_size;

	int 						_tcp_read_buffer_init_size_bytes;
	int 						_tcp_read_buffer_max_size_bytes;

	int 						_tcp_write_buffer_init_size_bytes;
	int 						_tcp_write_buffer_max_size_bytes;

	std::string 						_log_file_name_prefix;
	int 						_log_flush_interval_second;
	int 						_log_high_watermask;
	int 						_log_file_rolling;
	int 						_log_file_max_size_bytes;


public: // getter

	int getMaxTcpConnectionPerWorker() const;
	int getMaxTcpEvictionRule() const;
	int getInactiveTcpTimeOut() const;
	int getInactiveTcpEvictionRule() const;
	int getEvictionPoolSize() const;

private: // load balance functions

	Worker * loadBalanceRoundRobin(uint32_t ip);
	Worker * loadBalanceRandom(uint32_t ip);
	Worker * loadBalanceMinConnection(uint32_t ip);
	Worker * loadBalanceIPHash(uint32_t ip);
};

}

#endif
