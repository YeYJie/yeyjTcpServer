#include <sys/time.h>

int getTimeInSecond()
{
	timeval temp;
	gettimeofday(&temp, NULL);
	return temp.tv_sec;
}

int getTimeInMicroSecond()
{
	timeval temp;
	gettimeofday(&temp, NULL);
	return temp.tv_usec;
}
