#ifndef _MUTEX_H_
#define _MUTEX_H_

#include "include.h"

namespace yeyj
{

class MutexLock
{
public:

	MutexLock() : _holder(0) {
		pthread_mutex_init(&_mutex, NULL);
	}

	~MutexLock() {
		_holder = 0;
		pthread_mutex_destroy(&_mutex);
	}

	void lock() {
		pthread_mutex_lock(&_mutex);
	}

	void unlock() {
		pthread_mutex_unlock(&_mutex);
	}

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
