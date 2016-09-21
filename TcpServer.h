#ifndef TCPSERVER_H_
#define TCPSERVER_H_

#include <functional>
#include <netinet/in.h>
#include <string>
#include <vector>

#include "Acceptor.h"
#include "AsyncLogging.h"
#include "InetSockAddr.h"
#include "Worker.h"
#include "TcpConnection.h"

namespace yeyj
{


class TcpServer
{
public:

	explicit TcpServer(const int & port);

	~TcpServer();

	typedef std::function<void ()> 	ConnectionCallback;
	typedef std::function<void ()> 	MessageCallback;

	/*
	 *	runs the tcpserver with 'n' worker threads
	 * */
	void start(const int & n);

	void stop();

	void setConnectionCallback(const ConnectionCallback & cb);

	void setMessageCallback(const MessageCallback & cb);
	
private:

	/*
	 *  once a new connection is accept, this function will be called 
	 *  it finds a thread from the threadPool to work on that connection
	 * */
	void newConnection(int connSock, InetSockAddr peerAddr);

	Worker * findAWorker();

private:

	std::string 						_name;

	Acceptor 							_acceptor;

	std::vector<Worker *> 		_threadPool;

	ConnectionCallback 			_connectionCallback;
	MessageCallback 			_messageCallback;

};

}

#endif
