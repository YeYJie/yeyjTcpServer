#include "AsyncLogging.h"
#include <unistd.h>
#include <cassert>

using namespace yeyj;

AsyncLogging::AsyncLogging(const string & logfilename) :
	_logfilename(logfilename),
	_thread(std::bind(&AsyncLogging::threadFunction, this), "Log"),
	_running(false)
{
}

void AsyncLogging::start()
{
	_running = true;
	_thread.start();
}

void AsyncLogging::stop()
{
	_running = false;
	_thread.join();
}

void AsyncLogging::append(const string & logline)
{
	if(!_running) return;
	MutexLockGuard lock(_mutex);
	_currentBuffer.push_back(logline);
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

	void write(const string & data) {
		printf("[%s]\n", data.data());
		fwrite(data.c_str(), data.size(), 1, _file);
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
		sleep(1);
		{
			MutexLockGuard lock(_mutex);
			_currentBuffer.swap(_nextBuffer);
		}
		for(int i = 0; i < _nextBuffer.size(); ++i)
			logfile.write(_nextBuffer[i]);
		_nextBuffer.clear();
	}
	if(!_currentBuffer.empty()) {
		for(const string & s : _currentBuffer)
			logfile.write(s);
	}
}
