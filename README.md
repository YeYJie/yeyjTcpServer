# yeyjTcpServer

#### An echo server example :

~~~ c++

// main.cpp

#include <iostream>
#include "tcpServer.h"

// not necessary
void onConnection(const yeyj::TcpConnectionPtr & conn)
{
	std::cout << "new connection : " << conn->getId() << std::endl;
}

// echo
void onMessage(const yeyj::TcpConnectionPtr & conn)
{
	std::string data = conn->receiveRawAsString();
	conn->log(LOG_DEBUG, data);
	conn->sendRaw(data);
}

// not necessary
void onDisconnection(const yeyj::TcpConnectionPtr & conn)
{
	std::cout << "disconnection : " << conn->getId() << std::endl;
}

int main()
{
	yeyj::TcpServer server(44350);	// port
	
	server.setConnectionCallback(onConnection);		// not necessary
	server.setMessageCallback(onMessage);
	server.setDisconnectionCallback(onDisconnection);	// not necessary
	
	server.start(2);	// run with 2 worker threads
	
	return 0;
}
~~~

#### Build :

	make   # dependencies : pthread, tcmalloc c++11

#### Config File :

~~~
# some comments
# ...

max-tcp-= 100
max-tcp-per-worker = 100

# none | random | inactive
max-tcp-eviction-rule = inactive

inactive-tcp-timeout = 10

# none | close
inactive-tcp-eviction-rule = none

# nK | nM | nG
max-vm-memory = 100M

# nK | nM | nG
max-rss = 100M

eviction-pool-size = 5

# round-robin | random | min-connection | ip-hash
load-balance = ip-hash

# nK | nM | nG
tcp-read-buffer-init-size-bytes = 1M
tcp-read-buffer-max-size-bytes = 10M

# nK | nM | nG
tcp-write-buffer-init-size-bytes = 1M
tcp-write-buffer-max-size-bytes = 10M


log-file-name-prefix = yeyjlogfile

# none | size | daily | size & daily
log-file-rolling = size & daily
log-file-max-size = 1K

log-flush-interval-second = 1
log-high-watermask = 1000
~~~
