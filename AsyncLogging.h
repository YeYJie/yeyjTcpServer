
/*
 *	Usage : ( must link with 'AsyncLogging.cpp' ) 
 *
 *	void f() {
 *		.
 *		YEYJ_LOG("format_string", ...);
 *		.
 *	}
 *
 *	int main() {
 *		
 *		setGlobalLogName("xxx");
 *		startGlobalLog();
 *		[setGlobalLoggerFlushInterval(x);]
 *		[setGlobalLoggerHighWaterMask(x);]
 *		.
 *		
 *		f();
 *
 *		.
 *
 *		stopGlobalLog();
 *
 *	}
 *
 *
 * For every calls to YEYJ_LOG, it automatically add the corresponding file
 * name and line number
 *
 * May extends to support log level
 *
 */

#ifndef ASYNCLOGGING_H_
#define ASYNCLOGGING_H_

#include "Thread.h"
#include "Mutex.h"
#include "Condition.h"
#include <string>
#include <vector>

namespace  yeyj
{

class AsyncLogging
{
public:
	AsyncLogging(const std::string & logfilenamie = string(),
				 const int & flushInterval = 5,
				 const int & highWaterMask = 10000);

	void append(const std::string & logline);

	void start(); 

	void stop(); 

	/*
	 * flush the buffed logs to disk every 'second' seconds
	 * */
	void setFlushInterval(const int & second);

	/* 
	 * when the numbers of the buffered logs exceed 'highWaterMask'
	 * then flush then to disk
	 * */
	void setHighWaterMask(const int & highWaterMask);

	void setName(const std::string & newName);

private:

	void threadFunction();

private:

	typedef std::vector<std::string> 		Buffer;

	std::string 		_logfilename;

	int 				_flushInterval;
	int 				_highWaterMask;

	yeyj::Thread 		_thread;
	bool 				_running;

	MutexLock			_mutex;
	Condition 			_cond;

	Buffer 				_currentBuffer;
	Buffer 				_nextBuffer;

};

}

/* declared at 'AsyncLogging.cpp' */
extern yeyj::AsyncLogging yeyjGlobalLogger;

#define setGlobalLoggerName(newName)	yeyjGlobalLogger.setName(newName)
#define startGlobalLogging()			yeyjGlobalLogger.start()
#define stopGlobalLogging()				yeyjGlobalLogger.stop()
#define setGlobalLoggerFlushInterval(f) \
	yeyjGlobalLogger.setFlushInterval(f)
#define setGlobalLoggerHighWaterMask(h)	\
	yeyjGlobalLogger.setHighWaterMask(h)

#define YEYJ_LOG_FORMAT_CONCAT(a, b) a b

#define YEYJ_LOG(format, args...) 										\
	char yeyj_log_buffer[100];											\
	int yeyj_log_len = snprintf(yeyj_log_buffer, 100, YEYJ_LOG_FORMAT_CONCAT("%s %d  ", format), __FILE__, __LINE__, ## args); \
	yeyjGlobalLogger.append(string(yeyj_log_buffer, yeyj_log_len)); 		\

#endif
