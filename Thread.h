#ifndef THREAD_H_
#define THREAD_H_

#include <functional>
#include <pthread.h>
#include <string>

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
