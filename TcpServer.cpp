#include "TcpServer.h"
#include "AsyncLogging.h"

using namespace yeyj;


TcpServer::TcpServer(int port) : _acceptor(port)
{
	loadConfig("yeyj.config");

	_acceptor.setConnectionCallback(
		std::bind(&TcpServer::newConnection,
				  this,
				  std::placeholders::_1,
				  std::placeholders::_2));

	// setGlobalLoggerName(_log_file_name_prefix);
	// startGlobalLogging();
}

TcpServer::~TcpServer()
{
	stopGlobalLogging();
}

void TcpServer::setConnectionCallback(const ConnectionCallback & cb)
{
	_connectionCallback = cb;
}

void TcpServer::setMessageCallback(const MessageCallback & cb)
{
	_messageCallback = cb;
}

/*
 * 	find a worker to work on the new connection
 * 	consideration of work balance
 * */
// Worker * TcpServer::findAWorker()
// {
// 	/* round-robin... */
// 	static int index = 0;
// 	return _threadPool[(index++) % _threadPool.size()];
// }

Worker * TcpServer::loadBalanceRoundRobin()
{
	static int index = 0;
	return _threadPool[(index++) % _threadPool.size()];
}

Worker * TcpServer::loadBalanceRandom()
{
	int index = rand() % _threadPool.size();
	// cout << "loadBalanceRandom : " << index << endl;
	return _threadPool[index];
}

Worker * TcpServer::loadBalanceMinConnection()
{
	int index = 0;
	int connections = _threadPool[0]->getConnectionNum();
	for(int i = 1; i < _threadPool.size(); ++i) {
		if(_threadPool[i]->getConnectionNum() < connections) {
			index = i;
			connections = _threadPool[i]->getConnectionNum();
		}
	}
	return _threadPool[index];
}

void TcpServer::newConnection(int connSock, InetSockAddr peerAddr)
{
	// YEYJ_LOG("TcpServer::newConnection [%s] [%d]\n",
	// 		 peerAddr.getIpAsChar(),
	// 		 peerAddr.getPort());

	printf("TcpServer::newConnection [%s] [%d]\n",
			peerAddr.getIpAsChar(),
			peerAddr.getPort());

	Worker * worker = _loadBalance();

	worker->registerNewConnection(new TcpConnection(connSock, peerAddr,
							   						_connectionCallback,
							   						_messageCallback));

	// printf("TcpServer::newConnection\n");
}

/*
 *	starts all the worker threads and the acceptor
 * */
void TcpServer::start(const int & n)
{
	int maxConnection = min(_max_tcp_per_worker, _max_tcp / n);
	// for epoll, maxevents must be greater than 0
	if(maxConnection <= 0)
		maxConnection = 1;
	for(int i = 0; i < n; ++i) {
		/* FIXME 100 */
		_threadPool.push_back(new Worker(this));
		_threadPool[i]->setMaxConnection(maxConnection);
		_threadPool[i]->start();
	}
	_acceptor.start();
}

/*
 * 	stops all the worker threads and the acceptor
 * */
void TcpServer::stop()
{
	//_acceptor.stop();
	for(int i = 0; i < _threadPool.size(); ++i)
		_threadPool[i]->stop();
}


void TcpServer::loadConfig(const char * configFileName)
{
	cout << "loading config : " << configFileName << endl;

	ifstream configFile(configFileName, ios::in);
	if(configFile.is_open())
	{
		string line = "";
		while(true) {
			if(configFile.eof())
				break;
			getline(configFile, line);

			if(line.empty())
				continue;
			if(line[0] == '#')
				continue;

			vector<string> currentLine = split(line, "=");
			if(currentLine.size() != 2)
				break;

			string key = trim(currentLine[0]);
			string value = trim(currentLine[1]);


			if(key == "listening-port")
			{
				_listenning_port = atoi(value.data());
				cout << "config : listening-port " << _listenning_port << endl;
			}
			else if(key == "max-tcp")
			{
				_max_tcp = atoi(value.data());
				cout << "config : max-tcp" << " " << _max_tcp << endl;
			}
			else if(key == "max-tcp-per-worker")
			{
				_max_tcp_per_worker = atoi(value.data());
				cout << "config : max-tcp-per-worker" << " " << _max_tcp_per_worker << endl;
			}
			else if(key == "max-tcp-eviction-rule")
			{
				if(value == "none")
					_max_tcp_eviction_rule = MAX_TCP_EVICTION_NONE;
				else if(value == "random")
					_max_tcp_eviction_rule = MAX_TCP_EVICTION_RANDOM;
				else if(value == "inactive")
					_max_tcp_eviction_rule = MAX_TCP_EVICTION_INACTIVE;
				cout << "config : max-tcp-eviction-rule" << " " << value << endl;
			}
			else if(key == "inactive-tcp-timeout")
			{
				_inactive_tcp_timeout = atoi(value.data());
				cout << "config : inactive-tcp-timeout" << " " << _inactive_tcp_timeout << endl;
			}
			else if(key == "inactive-tcp-eviction-rule")
			{
				if(value == "none")
					_inactive_tcp_eviction_rule = INACTIVE_TCP_EVICTION_NONE;
				else if(value == "close")
					_inactive_tcp_eviction_rule = INACTIVE_TCP_EVICTION_CLOSE;
				cout << "config : inactive-tcp-eviction-rule" << " " << value << endl;
			}
			else if(key == "load-balance")
			{
				if(value == "round-robin")
					_loadBalance = std::bind(&TcpServer::loadBalanceRoundRobin, this);
					// _load_balance = LOAD_BALANCE_ROUNDROBIN;
				else if(value == "random")
					_loadBalance = std::bind(&TcpServer::loadBalanceRandom, this);
					// _load_balance = LOAD_BALANCE_RANDOM;
				else if(value == "minConnection")
					_loadBalance = std::bind(&TcpServer::loadBalanceMinConnection, this);
					// _load_balance = LOAD_BALANCE_MINCONNECTION;
				cout << "config : load-balance" << " " << value << endl;
			}
			else if(key == "tcp-read-buffer-init-size-bytes")
			{
				_tcp_read_buffer_init_size_bytes = atoi(value.data());
				cout << "config : tcp-read-buffer-size-bytes" << " " << _tcp_read_buffer_init_size_bytes << endl;
			}
			else if(key == "tcp-write-buffer-init-size-bytes")
			{
				_tcp_write_buffer_init_size_bytes = atoi(value.data());
				cout << "config : tcp-write-buffer-size-bytes" << " " << _tcp_write_buffer_init_size_bytes << endl;
			}
			else if(key == "tcp-read-buffer-max-size-bytes")
			{
				_tcp_read_buffer_max_size_bytes = atoi(value.data());
				cout << "config : tcp-read-buffer-size-bytes" << " " << _tcp_read_buffer_max_size_bytes << endl;
			}
			else if(key == "tcp-write-buffer-max-size-bytes")
			{
				_tcp_write_buffer_max_size_bytes = atoi(value.data());
				cout << "config : tcp-write-buffer-size-bytes" << " " << _tcp_write_buffer_max_size_bytes << endl;
			}
			else if(key == "log-file-name-prefix")
			{
				_log_file_name_prefix = value;
				cout << "config : log-file-name-prefix" << " " << _log_file_name_prefix << endl;
			}
			else if(key == "log-file-max-size")
			{
				_log_file_max_size = atoi(value.data());
				cout << "config : log-file-max-size" << " " << _log_file_max_size << endl;
			}
			else if(key == "log-flush-interval-second")
			{
				_log_flush_interval_second = atoi(value.data());
				cout << "config : log-flush-interval-second" << " " << _log_flush_interval_second << endl;
			}
			else if(key == "log-high-watermask")
			{
				_log_high_watermask = atoi(value.data());
				cout << "config : log-high-watermask" << " " << _log_high_watermask << endl;
			}
		}
	}
	cout << "loading config done..." << endl;
}


int TcpServer::getListenningPort() const {
	return _listenning_port;
}

int TcpServer::getMaxTcpConnection() const {
	return _max_tcp;
}

int TcpServer::getMaxTcpConnectionPerWorker() const {
	return _max_tcp_per_worker;
}

int TcpServer::getMaxTcpEvictionRule() const {
	return _max_tcp_eviction_rule;
}

int TcpServer::getInactiveTcpTimeOut() const {
	return _inactive_tcp_timeout;
}

int TcpServer::getInactiveTcpEvictionRule() const {
	return _inactive_tcp_eviction_rule;
}

int TcpServer::getReadBufferInitSize() const {
	return _tcp_read_buffer_init_size_bytes;
}

int TcpServer::getReadBufferMaxSize() const {
	return _tcp_read_buffer_max_size_bytes;
}

int TcpServer::getWriteBufferInitSize() const {
	return _tcp_write_buffer_init_size_bytes;
}

int TcpServer::getWriteBufferMaxSize() const {
	return _tcp_write_buffer_max_size_bytes;
}

string TcpServer::getLogFileNamePrefix() const {
	return _log_file_name_prefix;
}

int TcpServer::getLogFileNameSize() const {
	return _log_file_max_size;
}

int TcpServer::getLogFlushInterval() const {
	return _log_flush_interval_second;
}

int TcpServer::getLogHighWaterMask() const {
	return _log_high_watermask;
}