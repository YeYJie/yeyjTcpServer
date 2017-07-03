#include "worker.h"
using namespace yeyj;

Worker::Worker(TcpServer * master) :
	_master(master),
	Thread(std::bind(&Worker::workFunction, this), "worker"),
	_mutex()
{
	_epollfd = epoll_create(1);
}

Worker::~Worker()
{
	_connectionPool.clear();
	_master = nullptr;
}

void Worker::setMaxConnection(const int n)
{
	_maxConnection = n;
}

int Worker::getConnectionNum() const
{
	return _connectionPool.size();
}

void Worker::registerNewConnection(const TcpConnectionPtr & conn)
{
	MutexLockGuard lock(_mutex);
	_incomingConnection.push(conn);
}

void Worker::start()
{
	Thread::start();
}

void Worker::stop()
{
	Thread::join();
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

		for(int i = 0; i < nfds; ++i) {

			uint64_t connId = static_cast<uint64_t>(events[i].data.u64);
			TcpConnectionPtr conn = _connectionPool[connId];

			if(events[i].events & EPOLLERR
				|| events[i].events & EPOLLHUP)
			{
				epoll_ctl(_epollfd, EPOLL_CTL_DEL,
							conn->getfd(), conn->getEpollEvent());
				std::cout << "Worker::workFunction EPOLLERR or EPOLLHUP on "
					<< conn->getfd() << std::endl;
				removeConnection(conn);
			}

			if(events[i].events & EPOLLIN)
				conn->onReadableEvent();
			if(events[i].events & EPOLLOUT)
				conn->onWritableEvent();

			if(conn->close()) {
				removeConnection(conn);
			}
		}

		registerNewConnection();

		manageInactiveConnection();

		if(_connectionPool.size() > _master->getMaxTcpConnectionPerWorker()
			|| _master->exceedMaxMemory())
			eviction();
	}
}

void Worker::removeConnection(const TcpConnectionPtr & conn)
{
	epoll_ctl(_epollfd, EPOLL_CTL_DEL, conn->getfd(), conn->getEpollEvent());
	_connectionPool.erase(conn->getId());
}


void Worker::registerNewConnection()
{
	std::queue<TcpConnectionPtr> temp;
	{
		MutexLockGuard lock(_mutex);
		_incomingConnection.swap(temp);
	}

	while(!temp.empty()) {
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
		removeConnection(_connectionPool.getRandomValue());
	else if(rule == MAX_TCP_EVICTION_INACTIVE) {
		int numEviction = std::min(_connectionPool.size(),
									_master->getEvictionPoolSize());
		evictRandomN(numEviction, true);
	}
	else
		std::cout << "\n\ntcp server error eviction rule value\n" << std::endl;
}

void Worker::manageInactiveConnection()
{
	int rule = _master->getInactiveTcpEvictionRule();

	if(rule == INACTIVE_TCP_EVICTION_NONE) {
		// std::cout << "Worker::manageInactiveConnection do nothing" << std::endl;
		return;
	}

	evictRandomN(_master->getEvictionPoolSize(), false);
}

void Worker::evictRandomN(int n, bool force)
{
	if(n <= 0)
		return;
	int currentTime = getTimeInSecond();
	uint64_t mostInactiveId = 0xFFFFFFFFFFFFFFFF;
	int mostInactiveTime = 0;
	for(int i = 0; i < n; ++i)
	{
		if(_connectionPool.empty())
			return;
		const TcpConnectionPtr & conn = _connectionPool.getRandomValue();
		if(conn->close())
			removeConnection(conn);
		int inactiveTime = currentTime - conn->getLastActiveTime();
		if(inactiveTime > mostInactiveTime) {
			mostInactiveTime = inactiveTime;
			mostInactiveId = conn->getId();
		}
	}
	if(mostInactiveId != 0xFFFFFFFFFFFFFFFF
		&& (force || mostInactiveTime > _master->getInactiveTcpTimeOut()))
	{
		// printf("Worker::evictRandomN [%lld]\n", mostInactiveId);
		std::cout << "Worker::evictRandomN [" << mostInactiveId << "]" << std::endl;
		removeConnection(_connectionPool[mostInactiveId]);
	}
}