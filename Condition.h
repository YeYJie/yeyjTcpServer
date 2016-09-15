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

	void waitForSeconds(const int & second){
		struct timespec abstime;
		clock_gettime(CLOCK_REALTIME, &abstime);
		
		// printf("Condition [%ld]\n", abstime.tv_nsec);

		const int64_t NanoSecondsPerSecond = 1e9;
		int64_t nanoSeconds = static_cast<int64_t>(
					second * NanoSecondsPerSecond);
		
		abstime.tv_sec += static_cast<time_t>(
				(abstime.tv_nsec + nanoSeconds) / NanoSecondsPerSecond);
		abstime.tv_nsec += static_cast<long>(
				(abstime.tv_nsec + nanoSeconds) % NanoSecondsPerSecond);

		// printf("Condition [%ld]\n", abstime.tv_nsec);
		

		pthread_cond_timedwait(&_pcond, _mutex.getMutex(), &abstime);
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
