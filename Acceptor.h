#ifndef ACCEPTOR_H_
#define ACCEPTOR_H_

#include "InetSockAddr.h"
#include <netinet/in.h>
#include <functional>

namespace yeyj
{

/*
 *	Work flows :
 *
 *	creates a listen socket on the given port
 *
 *	uses epoll to wait for connection
 *
 *	accepts the incoming connection
 *
 *	calls the callback function from TcpServer
 *
 * */
class Acceptor
{
public:
	
	typedef std::function<void (int, const InetSockAddr &)> ConnectionCallback;

	explicit Acceptor(const int & port);

	~Acceptor();

	void setConnectionCallback(const ConnectionCallback & cb);

	/* once start, the acceptor will not stop */
	void start();

private:
	
	int 				_port;
	int 				_listenfd;
	int 				_epollfd;

	ConnectionCallback 	_connectionCallback;
};

}

#endif
