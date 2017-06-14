#include <stdlib.h>
#include <cstdio>
#include <unistd.h>
#include <iostream>
using namespace std;

int main(int argc, char ** argv)
{
	int n = atoi(argv[1]);
	cout << "n = " << n << endl;
	for(int i = 0; i < n; ++i) {
		int pid = fork();
		if(pid == 0) {
			printf("[%d]\n", getpid());
			execl("/bin/nc", "127.0.0.1", "44350", (char*)0);
		}
	}
}
