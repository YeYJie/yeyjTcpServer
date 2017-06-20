#include "threadPool.h"
using namespace yeyj;

ThreadPool::ThreadPool(const string & name) :
	_name(name),
	_running(false),
	_mutex(),
	_maxTasksNum(1000),
	_currentTasksNum(0),
	_notEmpty(_mutex),
	_notFull(_mutex)
{
}

ThreadPool::~ThreadPool()
{
	if(_running)
		stop();
}

void ThreadPool::setMaxTasksNum(const int & num)
{
	assert(num > 0);
	_maxTasksNum = num;
}

void ThreadPool::start(int numThreads)
{
	assert(numThreads > 0);
	_running = true;
	for(int i = 0; i < numThreads; ++i) {
		_threads.push_back(
				new Thread(std::bind(&ThreadPool::threadInitFunc, this),
					_name + std::to_string(i))
				);
		_threads[i]->start();
	}
	if(numThreads == 0)
		threadInitFunc();
}

void ThreadPool::stop()
{
	{
		MutexLockGuard lock(_mutex);
		// printf("ThreadPool::stop get lock\n");
		_running = false;
		_notEmpty.notifyAll();
		// there we have to release the lock
	}
	for(int i = 0; i < _threads.size(); ++i)
		_threads[i]->join();
}

void ThreadPool::runTask(const Task & t)
{
	if(_threads.empty()) {
		t();
	}
	else {
		MutexLockGuard lock(_mutex);

		while(_running && _currentTasksNum == _maxTasksNum) {
			_notFull.wait();
		}
		assert(_currentTasksNum < _maxTasksNum);
		_tasks.push_back(t);
		++_currentTasksNum;
		_notEmpty.notifyAll();
	}
}

ThreadPool::Task ThreadPool::takeATask()
{
	MutexLockGuard lock(_mutex);
	while(_running && _tasks.empty())
		_notEmpty.wait();

	Task res;
	if(_tasks.empty()) return res;

	res = _tasks.front();
	_tasks.pop_front();
	--_currentTasksNum;
	_notFull.notify();

	return res;
}

void ThreadPool::threadInitFunc()
{
	// for debug
	// printf("ThreadPool::threadInitFunc %ld\n", pthread_self());

	while(_running) {
		Task t = takeATask();
		// {
		// 	MutexLockGuard lock(_mutex);
		// 	printf("-%d-\n", _currentTasksNum);
		// }
		if(t) t();
	}

}
