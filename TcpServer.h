#ifndef TCPSERVER_H_
#define TCPSERVER_H_

#include <functional>

namespace yeyj
{


class TcpServer
{
public:

	explicit TcpServer(const int & port);

	~TcpServer();

	typedef std::function<void ()> 	ConnectionCallback;
	typedef std::function<void ()> 	MessageCallback;
	typedef sockaddr_in 			InetSockAddr;

	void start();

	void stop();

	void setConnectionCallback(const ConnectionCallback & cb);

	void setMessageCallback(const MessageCallback & cb);
	
private:

	/*
	 *  once a new connection is accept, this function will be called 
	 *  it finds a thread from the threadPool to work on that connection
	 * */
	void newConnection(int connSock, const InetSockAddr & peerAddr);

	void findAWorker();

private:

	string 						_name;

	Acceptor 					_acceptor;

	vector<WorkerThead *> 		_threadPool;

	ConnectionCallback 			_connectionCallback;
	MessageCallback 			_messageCallback;

};

}

#endif
