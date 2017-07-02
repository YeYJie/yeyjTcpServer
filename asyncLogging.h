#ifndef _ASYNCLOGGING_H_
#define _ASYNCLOGGING_H_

#include "include.h"
#include "thread.h"
#include "mutex.h"
#include "condition.h"


namespace  yeyj
{

#define LOG_FILE_ROLLING_NONE 0
#define LOG_FILE_ROLLING_SIZE 1
#define LOG_FILE_ROLLING_DAILY 2
#define LOG_FILE_ROLLING_SIZEDAILY 3

class AsyncLogging
{
public:
	AsyncLogging(const std::string & logfilenamie = "",
				 const int & flushInterval = 5,
				 const int & highWaterMask = 100);

	void append(const std::string & logline);

	void start();

	void stop();

	void setName(const std::string & newName);

	void setFlushInterval(const int & second);
	void setHighWaterMask(const int & highWaterMask);

	void setRolling(int rollingRule);
	void setLogFileMaxSize(int bytes);


private:

	void threadFunction();

	std::string getNextLogFileName();

private:

	typedef std::vector<std::string> 		Buffer;

	std::string 		_logfilename;

	int 				_flushInterval;
	int 				_highWaterMask;

	int 				_rollingRule;
	int 				_maxSizeBytes;
	int 				_accumulateBytes;
	std::string 		_currentDate;

	yeyj::Thread 		_thread;
	bool 				_running;

	MutexLock			_mutex;
	Condition 			_cond;

	Buffer 				_currentBuffer;
	Buffer 				_nextBuffer;

};

}


#endif
