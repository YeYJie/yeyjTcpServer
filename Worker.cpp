#include "Worker.h"
using namespace yeyj;

// Worker::Worker(const int & maxConnection) :
// 	Thread(std::bind(&Worker::workFunction, this), "worker"),
// 	_maxConnection(maxConnection),
// 	_mutex()
// {
// 	_epollfd = epoll_create(1);
// }

Worker::Worker(TcpServer * master) :
	_master(master),
	Thread(std::bind(&Worker::workFunction, this), "worker"),
	// _maxConnection(maxConnection),
	_mutex()
{
	_epollfd = epoll_create(1);
}

void Worker::registerNewConnection(TcpConnection * conn)
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
							  0);
		// printf("Worker::workFunction [%d]\n", nfds);
		// if(nfds < 0)
		// 	cout << std::strerror(errno) << endl;

		for(int i = 0; i < nfds; ++i) {

			TcpConnection * conn = (TcpConnection *)events[i].data.ptr;

			if(events[i].events & EPOLLERR
				|| events[i].events & EPOLLHUP)
			{
				epoll_ctl(_epollfd, EPOLL_CTL_DEL,
							conn->getfd(), conn->getEpollEvent());
				cout << "Worker::workFunction EPOLLERR or EPOLLHUP on "
					<< conn->getfd() << endl;
				// TODO : free the connection
			}

			if(events[i].events & EPOLLIN)
				conn->onReadableEvent();
			if(events[i].events & EPOLLOUT)
				conn->onWritableEvent();

			conn->updateEpollEvent();
			// if(conn->hasSomethingToWrite()) {
			epoll_ctl(_epollfd, EPOLL_CTL_MOD,
						conn->getfd(), conn->getEpollEvent());
			// }
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

		// conn->onConnection();
		if(_connectionPool.find(conn->getHash()) == _connectionPool.end()) {
			_connectionPool[conn->getHash()] = conn;
			conn->onConnection();
		}

		// printf("Worker[%d]::registerNewConnection [%x] [%d] on [%d]\n",
		// 		Thread::getTid(), conn, conn->getfd(), _epollfd);
		epoll_ctl(_epollfd,
				  EPOLL_CTL_ADD,
				  conn->getfd(),
				  conn->getEpollEvent());
	}

	// manageInactiveConnection();

	// if(_connectionPool.size() > _master->getMaxTcpConnectionPerWorker())
	// 	eviction();
}

void Worker::eviction()
{
	int rule = _master->getMaxTcpEvictionRule();

	if(rule == MAX_TCP_EVICTION_NONE)
		return;
	else if(rule == MAX_TCP_EVICTION_RANDOM)
	{
		int poolguy = rand() % _connectionPool.size();
		// _connectionPool.erase(_connectionPool.begin() + poolguy);
	}
	else if(rule == MAX_TCP_EVICTION_INACTIVE)
	{
		static const int poolguyNum = 5;
		static int poolguys[poolguyNum];
		for(int i = 0; i < poolguyNum; ++i)
			poolguys[i] = rand() % _connectionPool.size();

		// TODO : the pool guy is the most inactive one
		// int poolguy = findPoolGuy(poolguys, poolguyNum);
		int poolguy = poolguys[0];

		// _connectionPool.erase(_connectionPool.begin() + poolguy);
	}
	else
	{
		cout << "\n\ntcp server error eviction rule value\n" << endl;
	}
}

void Worker::manageInactiveConnection()
{
	int rule = _master->getInactiveTcpEvictionRule();

	if(rule == INACTIVE_TCP_EVICTION_NONE) {
		// cout << "Worker::manageInactiveConnection do nothing" << endl;
		return;
	}

	static const int candidatesNum = 10;
	int candidates[candidatesNum];
	for(int i = 0; i < candidatesNum; ++i)
		candidates[i] = rand() % _connectionPool.size();

	// TODO
}