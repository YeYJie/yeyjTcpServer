#include "TcpServer.h"
#include "AsyncLogging.h"
using namespace yeyj;

TcpServer::TcpServer(const int & port) :
	_acceptor(port)
{
	_acceptor.setConnectionCallback(
		std::bind(&TcpServer::newConnection, this, /* new connection */));
}

TcpServer::~TcpServer()
{

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
void TcpServer::findAWorker()
{
}

void TcpServer::newConnection(int connSock, const InetSockAddr & peerAddr)
{
	WorkerThead * worker = findAWorker();
	worker->work(/* the new connecion */);
}

/*
 *	starts all the worker threads and the acceptor
 * */
void TcpServer::start()
{
	for(int i = 0; i < _threadPool.size(); ++i)
		_threadPool[i]->start();
	_acceptor.start();
}

/*
 * 	stops all the worker threads and the acceptor
 * */
void TcpServer::stop()
{
	_acceptor.stop();
	for(int i = 0; i < _threadPool.size(); ++i)
		_threadPool[i]->stop();
}
