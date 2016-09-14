#ifndef MUTEX_H_
#define MUTEX_H_

#include <pthread.h>

namespace yeyj
{

class MutexLock
{
public:

	MutexLock();

	~MutexLock();

	void lock();

	void unlock();

	// used only by Condition !
	pthread_mutex_t * getMutex() {
		return &_mutex;
	}


private:
	pthread_mutex_t 	_mutex;
	pid_t				_holder;
};



class MutexLockGuard
{
public:

	explicit MutexLockGuard(MutexLock & mutex) :
		_mutex(mutex) 
	{
		mutex.lock();
	}
	
	~MutexLockGuard() {
		_mutex.unlock();
	}

private:
	MutexLock & _mutex;
};


}

#endif
