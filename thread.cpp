#include "thread.h"
using namespace yeyj;

Thread::Thread(const ThreadFunc & threadfunc,
				const string & name) :
	_name(name),
	_tid(0),
	_threadfunc(threadfunc)
{
}

Thread::~Thread()
{
	pthread_detach(_tid);
}

void * Thread::startThread(void * obj)
{
	Thread * thread = static_cast<Thread *>(obj);
	thread->_threadfunc();
	return NULL;
}

void Thread::start()
{
	// cout << _tid << endl;
	pthread_create(&_tid, NULL, &startThread, this);
	// cout << _tid << endl;
}

void Thread::join()
{
	pthread_join(_tid, NULL);
}
