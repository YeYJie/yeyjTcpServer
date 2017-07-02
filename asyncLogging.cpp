#include "asyncLogging.h"
#include <unistd.h>
#include <cassert>

using namespace yeyj;

AsyncLogging::AsyncLogging(const string & logfilename,
						   const int & flushInterval,
						   const int & highWaterMask) :
	_logfilename(logfilename),
	_flushInterval(flushInterval),
	_highWaterMask(highWaterMask),
	_thread(std::bind(&AsyncLogging::threadFunction, this), "Log"),
	_running(false),
	_mutex(),
	_cond(_mutex),
	_accumulateBytes(0),
	_currentDate(std::move(getDateAsString()))
{
}

void AsyncLogging::start()
{
	_running = true;
	_thread.start();
}

void AsyncLogging::stop()
{
	_cond.notifyAll();
	_running = false;
	_thread.join();
}

void AsyncLogging::append(const string & logline)
{
	if(!_running) return;

	MutexLockGuard lock(_mutex);
	_currentBuffer.push_back(logline);
	_accumulateBytes += logline.size();

	// cout << "AsyncLogging::append " << logline << endl;

	if(_currentBuffer.size() >= _highWaterMask
		|| _accumulateBytes > _maxSizeBytes)
	{
		// printf("AsyncLogging::append %d\n", _currentBuffer.size());
		_cond.notify();
	}
}

void AsyncLogging::setFlushInterval(const int & second)
{
	_flushInterval = second;
}

void AsyncLogging::setHighWaterMask(const int & highWaterMask)
{
	_highWaterMask = highWaterMask;
}

void AsyncLogging::setName(const std::string & newName)
{
	_logfilename = newName;
}

void AsyncLogging::setRolling(int rollingRule)
{
	_rollingRule = rollingRule;
}

void AsyncLogging::setLogFileMaxSize(int bytes)
{
	_maxSizeBytes = bytes;
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
		if(_file)
			fclose(_file);
	}

	LogFile & operator=(const string & name) {
		if(_file)
			fclose(_file);
		_file = fopen(name.data(), "a");
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
	FILE * 		_file = nullptr;
};

/*
 *		LogFile end
 * */


std::string AsyncLogging::getNextLogFileName()
{
	static int suffix = 0;
	return _logfilename + format("_%d", suffix++);
}

void AsyncLogging::threadFunction()
{
	LogFile logfile(getNextLogFileName());
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

		if(_rollingRule != LOG_FILE_ROLLING_NONE)
		{
			if(_rollingRule == LOG_FILE_ROLLING_SIZE
				&& _accumulateBytes > _maxSizeBytes)
			{
				{
					MutexLockGuard lock(_mutex);
					_accumulateBytes = 0;
				}
				logfile = getNextLogFileName();
			}
			else if(_rollingRule == LOG_FILE_ROLLING_DAILY
				&& getDateAsString() != _currentDate)
			{
				logfile = getNextLogFileName();
			}
			else if(_rollingRule == LOG_FILE_ROLLING_SIZEDAILY
				&& ( _accumulateBytes > _maxSizeBytes
					|| getDateAsString() != _currentDate))
			{
				{
					MutexLockGuard lock(_mutex);
					_accumulateBytes = 0;
				}
				logfile = getNextLogFileName();
			}
		}
	}
	if(!_currentBuffer.empty()) {
		for(const string & s : _currentBuffer)
			logfile.writeLine(s);
		logfile.flush();
	}
}
