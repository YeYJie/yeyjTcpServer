#ifndef _INCLUDE_H_
#define _INCLUDE_H_

#include <iostream>
#include <fstream>
#include <cstdio>

#include <unistd.h>
#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <errno.h>

#include <fcntl.h>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <string>
#include <strings.h>
#include <string.h>
#include <cstring>
#include <functional>
#include <vector>
#include <deque>
#include <queue>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <memory>

#include <pthread.h>

#include <netinet/in.h>
#include <arpa/inet.h>

int getTimeInSecond();
int getTimeInMilliSecond();
int getTimeInMicroSecond();

std::string & ltrim(std::string &s);
std::string & rtrim(std::string &s);
std::string & trim(std::string &s);
std::vector<std::string> split(const std::string & s,
								const std::string & delimiters);

std::string format(const char *fmt, ...);
std::string getDateAsString();

// process_mem_usage(double &, double &) - takes two doubles by reference,
// attempts to read the system-dependent data for a process' virtual memory
// size and resident set size, and return the results in KB.
//
// On failure, returns 0.0, 0.0
void process_mem_usage(double & vm_usage, double & resident_set);

#define LOG_DEBUG 	"DEBUG"
#define LOG_ERROR 	"ERROR"
#define LOG_INFO 	"INFO"
#define LOG_TRACE 	"TRACE"
#define LOG_WARN 	"WARN"

#endif