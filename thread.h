#ifndef _THREAD_H_
#define _THREAD_H_

#include "include.h"

using std::string;

namespace yeyj
{


class Thread
{
public:
	typedef std::function<void ()> ThreadFunc;

	explicit Thread(const ThreadFunc & theadfunc,
						const string & name = string());

	~Thread();

	void start();
	void join();

	pthread_t getTid() {
		return _tid;
	}

	string getName() {
		return _name;
	}

private:
	static void * startThread(void * obj);

private:
	string 		_name;
	pthread_t 	_tid;
	ThreadFunc	_threadfunc;

};



}

#endif
