#include "asyncLogging.h"
#include <unistd.h>
#include <cassert>

using namespace yeyj;

// AsyncLogging yeyjGlobalLogger;

AsyncLogging::AsyncLogging(const string & logfilename,
						   const int & flushInterval,
						   const int & highWaterMask) :
	_logfilename(logfilename),
	_flushInterval(flushInterval),
	_highWaterMask(highWaterMask),
	_thread(std::bind(&AsyncLogging::threadFunction, this), "Log"),
	_running(false),
	_mutex(),
	_cond(_mutex)
{
}

void AsyncLogging::start()
{
	_running = true;
	_thread.start();
	// YEYJ_LOG("Start Logging\n");
}

void AsyncLogging::stop()
{
	// YEYJ_LOG("Stop Logging\n");
	_cond.notifyAll();
	_running = false;
	_thread.join();
}

void AsyncLogging::append(const string & logline)
{
	if(!_running) return;

	MutexLockGuard lock(_mutex);
	_currentBuffer.push_back(logline);

	// cout << "AsyncLogging::append " << logline << endl;

	if(_currentBuffer.size() >= _highWaterMask){
		// printf("AsyncLogging::append %d\n", _currentBuffer.size());
		_cond.notify();
	}
}

void AsyncLogging::setFlushInterval(const int & second)
{
	// printf("AsyncLogging::setFlushInterval [%d]\n", second);
	_flushInterval = second;
}

void AsyncLogging::setHighWaterMask(const int & highWaterMask)
{
	// printf("AsyncLogging::setHighWaterMask [%d]\n", highWaterMask);
	_highWaterMask = highWaterMask;
}

void AsyncLogging::setName(const std::string & newName)
{
	_logfilename = newName;
}


/* *
 * 		LogFile begin
 * */

#include <stdio.h>
using namespace std;

class LogFile
{
public:

	LogFile(const string & name) {
		_file = fopen(name.data(), "a");
	}

	~LogFile() {
		fclose(_file);
	}

	void write(const string & line) {
		fwrite(line.c_str(), line.size(), 1, _file);
	}

	void writeLine(const string & data) {
		if(data.back() != '\n')
			write(data + "\n");
		else
			write(data);
	}

	void flush() {
		fflush(_file);
	}

private:
	FILE * 		_file;
};

/*
 *		LogFile end
 * */


void AsyncLogging::threadFunction()
{
	LogFile logfile(_logfilename);
	while(_running) {
		{
			MutexLockGuard lock(_mutex);

			// spurious wakeup ?
			if(_running && _currentBuffer.size() < _highWaterMask)
				_cond.waitForSeconds(_flushInterval);

			_currentBuffer.swap(_nextBuffer);
		}
		if(_nextBuffer.empty()) continue;

		// cout << "AsyncLogging::threadFunction " << _nextBuffer.size() << endl;
		for(int i = 0; i < _nextBuffer.size(); ++i)
			logfile.writeLine(_nextBuffer[i]);
		logfile.flush();
		_nextBuffer.clear();
	}
	if(!_currentBuffer.empty()) {
		for(const string & s : _currentBuffer)
			logfile.writeLine(s);
		logfile.flush();
	}
}
