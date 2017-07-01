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

using namespace std;

int getTimeInSecond();
int getTimeInMilliSecond();
int getTimeInMicroSecond();

string & ltrim(string &s);
string & rtrim(string &s);
string & trim(string &s);
vector<string> split(const string & s, const string & delimiters);

std::string format(const char *fmt, ...);

// process_mem_usage(double &, double &) - takes two doubles by reference,
// attempts to read the system-dependent data for a process' virtual memory
// size and resident set size, and return the results in KB.
//
// On failure, returns 0.0, 0.0
void process_mem_usage(double & vm_usage, double & resident_set);

#endif