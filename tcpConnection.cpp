#include "tcpConnection.h"
using namespace yeyj;

TcpConnection::TcpConnection(uint64_t id,
							 const int & connfd,
							 const InetSockAddr & peerAddr,
							 TcpServer * master,
							 int read_buffer_init_size,
							 int read_buffer_max_size,
							 int write_buffer_init_size,
							 int write_buffer_max_size) :
	_id(id),
	_connfd(connfd),
	_peerAddr(peerAddr),
	_master(master),
	_readBuffer(read_buffer_init_size, read_buffer_max_size),
	_writeBuffer(write_buffer_init_size, write_buffer_max_size),
	_close(false)
{
	/*	set the connection socket as non-blocking socket
	 * */
	int flags = fcntl(_connfd, F_GETFL, 0);
	assert(flags >= 0);
	assert(fcntl(_connfd, F_SETFL, flags | O_NONBLOCK) >= 0);

	int shit = 1;
	setsockopt(_connfd, SOL_SOCKET, SO_KEEPALIVE,
			   &shit, sizeof(int));

	_epollEvent.events = 0;
	_epollEvent.events |= EPOLLIN | EPOLLOUT | EPOLLET;
	// _epollEvent.data.ptr = this;
	_epollEvent.data.u64 = _id;

	_lastActiveTime = getTimeInSecond();
	// printf("TcpConnection::constructor\n");
}

TcpConnection::~TcpConnection()
{

}

void TcpConnection::send(const std::string & str)
{
	if(_writeBuffer.getMaxSpace() < str.size()) {
		cout << "TcpConnection::send : str is too long";
	}
	_writeBuffer.write(str);
}

void TcpConnection::send(const char * str)
{
	send(str, strlen(str));
}

void TcpConnection::send(const char * str, int size)
{
	if(_writeBuffer.getMaxSpace() < size) {
		cout << "TcpConnection::send : str is too long";
	}
	_writeBuffer.write(str, size);
}

std::string TcpConnection::receiveAsString()
{
	return _readBuffer.readAsString();
}

std::string TcpConnection::receiveAsString(int length)
{
	return _readBuffer.readAsString(length);
}

int TcpConnection::receive(char * dst)
{
	return _readBuffer.read(dst);
}

int TcpConnection::receive(char * dst, int length)
{
	return _readBuffer.read(dst, length);
}

// int TcpConnection::getfd()
// {
// 	return _connfd;
// }

// epoll_event * TcpConnection::getEpollEvent()
// {
// 	return &_epollEvent;
// }

void TcpConnection::onConnection()
{
	// cout << "new tcp connection" << endl;
	// _connectionCallback(*this);
	_master->connectionCallback(shared_from_this());
}

void TcpConnection::onReadableEvent()
{
	_lastActiveTime = getTimeInSecond();

	int space = _readBuffer.getMaxSpace();
	if(space <= 0) {
		cout << "\n\nTcpConnection::onReadableEvent read buffer"
				" can not expand any more\n" << endl;
		exit(0);
	}
	char * buffer = new char[space]{0};
	// int len = -1;
	// len = read(_connfd, buffer, space);

	int ret = 0;
	int totalRead = 0;
	while((ret = read(_connfd, buffer + totalRead, space)) > 0) {
		totalRead += ret;
		space -= ret;
	}

	if(ret < 0 && errno != EAGAIN && errno != EWOULDBLOCK
		&& errno != ECONNABORTED && errno != EPROTO && errno != EINTR)
		cout << "\n\nTcpConnection::onReadableEvent : fatal error" << endl;
	// cout << "ondata [" << len << "]" << endl;

	if(totalRead <= 0)
		onDisconnection();
	else {
		_readBuffer.write(buffer, totalRead);

		// _readBuffer.read();
		onMessage();
	}
	delete[] buffer;
}

void TcpConnection::onWritableEvent()
{
	_lastActiveTime = getTimeInSecond();

	int size = _writeBuffer.getSize();
	if(size <= 0) {
		// cout << "TcpConnection::onWritableEvent nothing to write" << endl;
		return;
	}

	// cout << "TcpConnection::onWritableEvent() [" << size << "]" << endl;

	char * buffer = new char[size]{0};
	// int len = -1;
	_writeBuffer.copy(buffer, size);
	// len = write(_connfd, buffer, size);

	int ret = 0;
	int totalWrite = 0;
	while((ret = write(_connfd, buffer + totalWrite, size)) > 0) {
		totalWrite += ret;
		size -= ret;
	}
	if(ret < 0 && errno != EAGAIN && errno != EWOULDBLOCK
		&& errno != ECONNABORTED && errno != EPROTO && errno != EINTR)
		cout << "\n\nTcpConnection::onWritableEvent : fatal error" << endl;

	if(totalWrite <= 0) {
		cout << "TcpConnection::onWritableEvent len <= 0" << endl;
	}
	else {
		// printf("TcpConnection::onWritableEvent [%d]\n", len);
		_writeBuffer.forward(totalWrite);
	}
	delete[] buffer;
}

void TcpConnection::onDisconnection()
{
	// cout << "tcp connection close" << endl;
	// _disconnectionCallback(*this);
	_master->disconnectionCallback(shared_from_this());
	_close = true;
	// assert(close(_connfd) == 0);
}

void TcpConnection::onMessage()
{
	_master->messageCallback(shared_from_this());
}
