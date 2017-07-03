#ifndef _THREAD_H_
#define _THREAD_H_

#include "include.h"

namespace yeyj
{

typedef std::function<void ()> ThreadFunc;

class Thread
{
public:

	explicit Thread(const ThreadFunc & threadfunc,
						const std::string & name = "")
		:	_name(name),
			_tid(0),
			_threadfunc(threadfunc) { }

	~Thread() {
		pthread_detach(_tid);
	}

	Thread(const Thread &) = delete;
	Thread & operator=(const Thread &) = delete;

	void start() {
		pthread_create(&_tid, NULL, &startThread, this);
	}

	void join() {
		pthread_join(_tid, NULL);
	}

	pthread_t getTid() {
		return _tid;
	}

	std::string getName() {
		return _name;
	}

private:

	// used by pthread_create
	static void * startThread(void * obj)
	{
		Thread * thread = static_cast<Thread *>(obj);
		thread->_threadfunc();
		return NULL;
	}

private:
	std::string 		_name;
	pthread_t 	_tid = 0;
	ThreadFunc	_threadfunc;

};



}

#endif
