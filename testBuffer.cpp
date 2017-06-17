#include "buffer.h"
using namespace yeyj;

#define DST_SIZE 100

void clear(char * buf)
{
	for(int i = 0; i < DST_SIZE; ++i) buf[i] = 0;
}

int main()
{
	Buffer buffer(10, 20);
	char dst[DST_SIZE];
	clear(dst);

	buffer.print();
	buffer.write("yeyongjieyeyongjie", 18);
	buffer.print();

	buffer.read(dst, 1);
	buffer.read(dst + 1, 1);
	buffer.read(dst + 2, 1);
	buffer.read(dst + 3, 1);
	buffer.read(dst + 4, 1);

	cout << "[" << dst << "]" << endl;
	clear(dst);

	buffer.read(dst, 1);
	// cout << dst << endl;
	cout << "[" << dst << "]" << endl;
	clear(dst);

	buffer.write("haha", 4);
	buffer.print();

	buffer.write("z", 1);
	buffer.print();

	buffer.write("lll", 3);
	buffer.print();

	buffer.write("mmm", 3);
	buffer.print();

	// buffer.read(dst);
	// cout << "[" << dst << "]" << endl;
	// clear(dst);
	// buffer.read(dst);
	// cout << "[" << dst << "]" << endl;

	cout << buffer.readAsString() << endl;

	buffer.print();

	buffer.clear();
	buffer.write("ye");
	buffer.print();
	buffer.write("yong");
	buffer.print();
}