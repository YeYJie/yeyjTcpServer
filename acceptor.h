#ifndef _ACCEPTOR_H_
#define _ACCEPTOR_H_

#include "include.h"
#include "inetSockAddr.h"

namespace yeyj
{

class Acceptor
{
public:

	typedef std::function<void (int, const InetSockAddr &)> ConnectionCallback;
	typedef std::function<void ()> ServerCron;

	Acceptor() = default;

	explicit Acceptor(const int & port);

	~Acceptor();

	void setConnectionCallback(const ConnectionCallback & cb);

	void setServerCron(const ServerCron & sc);

	/* once start, the acceptor will not stop */
	void start(int timeout);

private:

	int 				_port;
	int 				_listenfd;
	int 				_epollfd;

	ConnectionCallback 	_connectionCallback;
	ServerCron 			_serverCron;
};

}

#endif
