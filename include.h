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

// class TcpConnection;

#endif