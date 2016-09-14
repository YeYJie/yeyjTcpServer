#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <functional>
#include <string>
#include <vector>
#include <deque>
#include "Thread.h"
#include "Mutex.h"
#include "Condition.h"

using std::string;

namespace yeyj
{

class ThreadPool
{
public:
	
	typedef std::function<void ()> Task;

	explicit ThreadPool(const string & name = string("ThreadPool"));

	~ThreadPool();

	void setMaxTasksNum(const int & num = 100);

	void start(int numThreads = 1);

	void stop();

	void runTask(const Task & t);

	string getName() {
		return _name;
	}

	void threadInitFunc();
private:
	

	Task takeATask();

private:
	
	string 					_name;

	bool 					_running;

	MutexLock 				_mutex;
	Condition 				_notEmpty;
	Condition 				_notFull;

	int 					_maxTasksNum;
	int 					_currentTasksNum;
	
	std::vector<Thread *> 	_threads;
	std::deque<Task> 		_tasks;

};


}

#endif
