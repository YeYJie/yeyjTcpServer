#include <iostream>
#include "Thread.h"
using namespace std;
using namespace yeyj;

void shit()
{
	for(int i = 0; i < 10000; ++i) {
		cout << i << " ";
	}
	cout << endl;
}

int main()
{
	Thread t(bind(&shit), "shit");
	cout << t.getName() << endl;

	Thread tt(bind(&shit), "fuck");
	cout << tt.getName() << endl;

	t.start();
	tt.start();

	t.join();
	tt.join();

	return 0;
}
