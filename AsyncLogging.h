#ifndef ASYNCLOGGING_H_
#define ASYNCLOGGING_H_

#include "Buffer.h"
#include "Thread.h"
#include "Mutex.h"
#include <string>
#include <vector>

namespace  yeyj
{

class AsyncLogging
{
public:
	AsyncLogging(const std::string & logfilename);

	void append(const std::string & logline);

	void start(); 

	void stop(); 

private:

	void threadFunction();

private:

	typedef std::vector<std::string> 		Buffer;

	std::string 		_logfilename;
	yeyj::Thread 		_thread;
	bool 				_running;

	MutexLock			_mutex;
	
	Buffer 				_currentBuffer;
	Buffer 				_nextBuffer;

};

}

#endif
