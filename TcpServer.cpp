#include "TcpServer.h"
#include "AsyncLogging.h"
#include <stdio.h>
using namespace yeyj;


TcpServer::TcpServer(const int & port) :
	_acceptor(port)
{
	_acceptor.setConnectionCallback(
		std::bind(&TcpServer::newConnection,
				  this,
				  std::placeholders::_1,
				  std::placeholders::_2));

	setGlobalLoggerName("TcpServer");
	startGlobalLogging();
}

TcpServer::~TcpServer()
{
	stopGlobalLogging();
}

void TcpServer::setConnectionCallback(const ConnectionCallback & cb)
{
	_connectionCallback = cb;
}

void TcpServer::setMessageCallback(const MessageCallback & cb)
{
	_messageCallback = cb;
}

/*
 * 	find a worker to work on the new connection
 * 	consideration of work balance
 * */
Worker * TcpServer::findAWorker()
{
	return _threadPool[0];
}

void TcpServer::newConnection(int connSock, InetSockAddr peerAddr)
{
	// YEYJ_LOG("TcpServer::newConnection [%s] [%d]\n",
	// 		 peerAddr.getIpAsChar(),
	// 		 peerAddr.getPort());
	// printf("TcpServer::newConnection [%s] [%d]\n",
	// 		peerAddr.getIpAsChar(),
	// 		peerAddr.getPort());
	Worker * worker = findAWorker();

	worker->work(new TcpConnection(connSock, peerAddr,
							   _connectionCallback,
							   _messageCallback));

	// printf("TcpServer::newConnection\n");
}

/*
 *	starts all the worker threads and the acceptor
 * */
void TcpServer::start(const int & n)
{
	for(int i = 0; i < n; ++i) {
		/* FIXME 100 */
		_threadPool.push_back(new Worker(100));
		_threadPool[i]->start();
	}
	_acceptor.start();
}

/*
 * 	stops all the worker threads and the acceptor
 * */
void TcpServer::stop()
{
	//_acceptor.stop();
	//for(int i = 0; i < _threadPool.size(); ++i)
	//	_threadPool[i]->stop();
}
