#ifndef CONDITION_H_
#define CONDITION_H_

#include "Mutex.h"
#include <pthread.h>

namespace yeyj
{

class Condition
{

public:

	explicit Condition(MutexLock & mutex) :
		_mutex(mutex) 
	{
		pthread_cond_init(&_pcond, NULL);	
	}

	~Condition() {
		pthread_cond_destroy(&_pcond);
	}

	void wait() {
		pthread_cond_wait(&_pcond, _mutex.getMutex());
	}

	void notify() {
		pthread_cond_signal(&_pcond);
	}
	
	void notifyAll() {
		pthread_cond_broadcast(&_pcond);
	}

private:
	
	MutexLock & 		_mutex;
	pthread_cond_t 		_pcond;
};


}

#endif
