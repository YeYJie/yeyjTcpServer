#include "TcpServer.h"
#include <iostream>
using namespace yeyj;
using namespace std;

int main()
{
	TcpServer server(44350);
	server.start(1);
	return 0;
}
