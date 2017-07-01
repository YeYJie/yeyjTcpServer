#ifndef _INETSOCKADDR_H_
#define _INETSOCKADDR_H_

#include "include.h"

namespace yeyj
{

class InetSockAddr
{
public:

	InetSockAddr(const short family = AF_INET,
						  const unsigned long addr = INADDR_ANY,
						  const unsigned short port = 0)
	{
		bzero((void *)&_addr, sizeof(_addr));
		_addr.sin_family = family;
		_addr.sin_port = htons((unsigned short)port);
		_addr.sin_addr.s_addr = htonl(addr);
	}

	InetSockAddr(const sockaddr_in & sockAddrIn) {
		_addr = sockAddrIn;
	}

	int getFamily() {
		return _addr.sin_family;
	}

	uint32_t getIP() {
		return _addr.sin_addr.s_addr;
	}

	uint16_t getPort() {
		return _addr.sin_port;
	}

	char * getIpAsChar() {
		return inet_ntoa(_addr.sin_addr);
	}

	std::string getIpAsString() {
		return std::string(inet_ntoa(_addr.sin_addr));
	}

private:

	struct sockaddr_in 		_addr;

};

}

#endif
