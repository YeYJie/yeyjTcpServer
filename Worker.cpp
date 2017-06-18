#include "Worker.h"
using namespace yeyj;

Worker::Worker(TcpServer * master) :
	_master(master),
	Thread(std::bind(&Worker::workFunction, this), "worker"),
	// _maxConnection(maxConnection),
	_mutex()
{
	_epollfd = epoll_create(1);
	_connectionPool.max_load_factor(5);
}

void Worker::registerNewConnection(const TcpConnectionPtr & conn)
{
	/*
	 *	we should put the new connection into a blocking queue instead of
	 *	manipulating the epoll fd, because while the worker is blocking on
	 *	epoll_wait, the behaviour would be undefined if the master thread
	 *	registers fd on that epoll instance
	 * */
	MutexLockGuard lock(_mutex);
	_incomingConnection.push(conn);
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

			TcpConnection * raw = (TcpConnection *)events[i].data.ptr;
			TcpConnectionPtr conn = raw->shared_from_this();

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

			if(!conn->close()) {
				conn->updateEpollEvent();
				epoll_ctl(_epollfd, EPOLL_CTL_MOD,
							conn->getfd(), conn->getEpollEvent());
			}
			else {
				removeConnection(conn);
			}
		}
		/* if there are incoming connections on the blocking queue,
		 * registers them on the epoll instance
		 * */
		registerNewConnection();

		manageInactiveConnection();

		if(_connectionPool.size() > _master->getMaxTcpConnectionPerWorker())
			eviction();
	}
}

void Worker::removeConnection(const TcpConnectionPtr & conn)
{
	epoll_ctl(_epollfd, EPOLL_CTL_DEL, conn->getfd(), conn->getEpollEvent());
	// _connectionPool.erase(remove(_connectionPool.begin(), _connectionPool.end(), conn));
	_connectionPool.erase(conn->getId());
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

	std::queue<TcpConnectionPtr> temp;
	{
		MutexLockGuard lock(_mutex);
		_incomingConnection.swap(temp);
		// if(!temp.empty() && _incomingConnection.empty())
		// 	printf("Worker::registerNewConnection not empty\n");
	}

	while(!temp.empty()) {
		// _connectionPool.push_back(std::move(temp.front()));
		// const TcpConnectionPtr & conn = _connectionPool.back();
		const TcpConnectionPtr & conn = temp.front();
		conn->onConnection();
		epoll_ctl(_epollfd, EPOLL_CTL_ADD,
					conn->getfd(), conn->getEpollEvent());
		_connectionPool[conn->getId()] = std::move(temp.front());
		temp.pop();
	}
}

void Worker::eviction()
{
	int rule = _master->getMaxTcpEvictionRule();

	if(rule == MAX_TCP_EVICTION_NONE)
		return;
	else if(rule == MAX_TCP_EVICTION_RANDOM)
		removeConnection(getRandomConnection());
	else if(rule == MAX_TCP_EVICTION_INACTIVE)
		evictRandomN(_master->getEvictionPoolSize());
	else
		cout << "\n\ntcp server error eviction rule value\n" << endl;
}

void Worker::manageInactiveConnection()
{
	// if(_connectionPool.empty())
	// 	cout << "connectionPool is empty" << endl;
	// else {
	// 	for(auto & i : _connectionPool)
	// 		cout << i.second.use_count() << " ";
	// 	cout << endl;
	// }

	int rule = _master->getInactiveTcpEvictionRule();

	if(rule == INACTIVE_TCP_EVICTION_NONE) {
		// cout << "Worker::manageInactiveConnection do nothing" << endl;
		return;
	}

	evictRandomN(_master->getEvictionPoolSize());
}

void Worker::evictRandomN(int n)
{
	int currentTime = getTimeInSecond();
	long long mostInactiveId = -1;
	int mostInactiveTime = 0;
	for(int i = 0; i < n; ++i)
	{
		if(_connectionPool.empty())
			return;
		const TcpConnectionPtr & conn = getRandomConnection();
		if(conn->close())
			removeConnection(conn);
		int inactiveTime = currentTime - conn->getLastActiveTime();
		if(inactiveTime > mostInactiveTime) {
			mostInactiveTime = inactiveTime;
			mostInactiveId = conn->getId();
		}
	}
	if(mostInactiveId >= 0
		&& mostInactiveTime > _master->getInactiveTcpTimeOut())
	{
		printf("Worker::evictRandomN [%lld]\n", mostInactiveId);
		removeConnection(_connectionPool[mostInactiveId]);
	}
}

const TcpConnectionPtr & Worker::getRandomConnection()
{
	int bucketIndex = rand() % _connectionPool.bucket_count();
	auto iter = _connectionPool.begin(bucketIndex);
	int elementIndex = rand() % _connectionPool.bucket_size(bucketIndex);
	while(elementIndex--)
		++iter;
	return iter->second;
}