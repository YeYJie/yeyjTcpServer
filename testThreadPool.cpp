#include <iostream>
#include <unistd.h>
#include <functional>
#include "threadPool.h"
using namespace std;
using namespace yeyj;

void shit()
{
	sleep(1);
	printf("[%d]\n", int(pthread_self() % 1000));
}

int main()
{
	ThreadPool pool;
	auto f = bind(&shit);

	pool.start(4);
	pool.setMaxTasksNum(10000);

	for(int i = 0; i < 10; ++i)
		pool.runTask(f);

	sleep(5);
	pool.stop();

	return 0;
}
