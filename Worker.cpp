#include "Worker.h"
#include <sys/epoll.h>
using namespace yeyj;

Worker::Worker(const int & maxConnection) :
	Thread(std::bind(&Worker::workFunction, this), "worker"),
	_maxConnection(maxConnection),
	_mutex()
{
	_epollfd = epoll_create(1); 
}

void Worker::work(TcpConnection * conn)
{
	/*	
	 *	we should put the new connection into a blocking queue instead of
	 *	manipulating the epoll fd, because while the worker is blocking on
	 *	epoll_wait, the behaviour would be undefined if the master thread
	 *	registers fd on that epoll instance
	 * */
	MutexLockGuard lock(_mutex);
	_incomingConnection.push(conn);
	// printf("Worker::work [%x][size:%d]\n", 
	// 		conn, _incomingConnection.size());
}

void Worker::start()
{
	Thread::start();
}

void Worker::stop()
{
	// Thread::stop();
}

void Worker::workFunction()
{
	struct epoll_event events[_maxConnection];
	while(true) {
		int nfds = epoll_wait(_epollfd,
							  events,
							  _maxConnection,
							  2000);
		// printf("Worker::workFunction [%d]\n", nfds);
		for(int i = 0; i < nfds; ++i) {
			
			/*	here we should check the value of fired fds' events field,
			 *	if some errors happened to a connection, we should close
			 *	that connection
			 * */
			// if((events[i].events & EPOLLERR)
			//    || (events[i].events & EPOLLHUP)
			//    || (!(events[i].events & EPOLLIN))) 
			// {
			// 	/* some thing wrong with that connection */
			// 	printf("Worker::workFunction connection error\n");
			// }	
			if(events[i].events & EPOLLERR)
				printf("Worker::workFunction EPOLLERR");

			if(events[i].events & EPOLLHUP)
				printf("Worker::workFunction EPOLLHUP");

			TcpConnection * conn = (TcpConnection *)events[i].data.ptr;
			conn->handleRead();
		}
		/* if there are incoming connections on the blocking queue,
		 * registers them on the epoll instance
		 * */
		registerNewConnection();
	}
}

/*	new connection from master thread would be stored temporaily
 *	on _incomingConnection, in this function, we register them on the
 *	worker's epoll instance
 * */
void Worker::registerNewConnection()
{
	// printf("Worker::registerNewConnection\n");
	
	// printf("Worker::registerNewConnection [size:%d]\n",
	// 		_incomingConnection.size());

	std::queue<TcpConnection *> temp;
	{
		MutexLockGuard lock(_mutex);
		_incomingConnection.swap(temp);
		// if(!temp.empty() && _incomingConnection.empty())
		// 	printf("Worker::registerNewConnection not empty\n");
	}
	
	// if(temp.size())
	// 	printf("Worker::registerNewConnection temp[size:%d]\n",
	// 			temp.size());

	while(!temp.empty()) {
		TcpConnection * conn = temp.front();
		temp.pop();
		// printf("Worker::registerNewConnection [%x] [%d] on [%d]\n", 
		// 		conn, conn->getfd(), _epollfd);
		epoll_ctl(_epollfd,
				  EPOLL_CTL_ADD,
				  conn->getfd(),
				  conn->getEpollEvent());
	}
}
