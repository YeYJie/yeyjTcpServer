#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include "asyncLogging.h"
#include "thread.h"
#include <string>
using namespace std;
using namespace yeyj;

// yeyj::AsyncLogging * g_log;

void threadfunc()
{
	for(int i = 0; i < 10000; ++i) {
		YEYJ_LOG("tid [%ld] n [%d]\n", pthread_self(), i);
	}

	// int n = 10;
	// char buffer[20];
	// int len = snprintf( buffer, 20, "%ld\n", pthread_self() );
	// while(n--)
	// 	g_log->append(string(buffer, len));
}

int main()
{
	// g_log = new AsyncLogging("shit");
	// g_log->start();

	setGlobalLoggerName("shit");
	setGlobalLoggerFlushInterval(2);
	setGlobalLoggerHighWaterMask(100);
	startGlobalLogging();

	Thread a(bind(&threadfunc));
	Thread b(bind(&threadfunc));
	Thread c(bind(&threadfunc));

	a.start();
	b.start();
	c.start();

	sleep(5);

	// g_log->stop();
	stopGlobalLogging();

	return 0;
}
