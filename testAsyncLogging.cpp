#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include "AsyncLogging.h"
#include "Thread.h"
#include <string>
using namespace std;
using namespace yeyj;

yeyj::AsyncLogging * g_log;

void threadfunc()
{
	int n = 10;
	char buffer[20];
	int len = snprintf( buffer, 20, "%ld\n", pthread_self() );
	while(n--)
		g_log->append(string(buffer, len));
}

int main()
{
	g_log = new AsyncLogging("shit");
	
	g_log->start();
	
	Thread a(bind(&threadfunc));
	Thread b(bind(&threadfunc));
	Thread c(bind(&threadfunc));

	a.start();
	b.start();
	c.start();

	sleep(5);

	g_log->stop();
	
	return 0;
}
