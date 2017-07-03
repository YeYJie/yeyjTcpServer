#include "tcpServer.h"
#include "asyncLogging.h"

using namespace yeyj;


TcpServer::TcpServer(int port) : _acceptor(port), _name("master")
{
	loadConfig("yeyj.config");

	updateTime();

	_logger.setName(_log_file_name_prefix);
	_logger.setFlushInterval(_log_flush_interval_second);
	_logger.setHighWaterMask(_log_high_watermask);
	_logger.setRolling(_log_file_rolling);
	_logger.setLogFileMaxSize(_log_file_max_size_bytes);
	_logger.start();

	_tid = pthread_self();

	_acceptor.setConnectionCallback(
		std::bind(&TcpServer::newConnection,
				  this,
				  std::placeholders::_1,
				  std::placeholders::_2));
	_acceptor.setServerCron(std::bind(&TcpServer::serverCron, this));
}

TcpServer::~TcpServer()
{
}

void TcpServer::setConnectionCallback(const ConnectionCallback & cb)
{
	connectionCallback = cb;
}

void TcpServer::setDisconnectionCallback(const DisconnectionCallback & cb)
{
	disconnectionCallback = cb;
}

void TcpServer::setMessageCallback(const MessageCallback & cb)
{
	messageCallback = cb;
}


Worker * TcpServer::loadBalanceRoundRobin(uint32_t ip)
{
	static int index = 0;
	return _threadPool[(index++) % _threadPool.size()];
}

Worker * TcpServer::loadBalanceRandom(uint32_t ip)
{
	int index = rand() % _threadPool.size();
	// std::cout << "loadBalanceRandom : " << index << std::endl;
	return _threadPool[index];
}

Worker * TcpServer::loadBalanceMinConnection(uint32_t ip)
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

Worker * TcpServer::loadBalanceIPHash(uint32_t ip)
{
	static auto hashFunction = std::hash<uint32_t>();
	return _threadPool[hashFunction(ip) % _threadPool.size()];
}

void TcpServer::newConnection(int connSock, InetSockAddr peerAddr)
{
	printf("TcpServer::newConnection [%d] [%s] [%d]\n",
			connSock,
			peerAddr.getIpAsChar(),
			peerAddr.getPort());

	Worker * worker = _loadBalance(peerAddr.getIP());

	static uint64_t id = 0;

	worker->registerNewConnection(
		std::make_shared<TcpConnection>(++id, connSock, peerAddr,
									this,
									worker,
									_tcp_read_buffer_init_size_bytes,
									_tcp_read_buffer_max_size_bytes,
									_tcp_write_buffer_init_size_bytes,
									_tcp_write_buffer_max_size_bytes)
								);
}

void TcpServer::serverCron()
{
	// std::cout << "TcpServer::serverCron" << std::endl;
	updateTime();
	checkMaxMemory();
}

void TcpServer::checkMaxMemory()
{
	double vm_usage = 0, resident_set = 0;
	process_mem_usage(vm_usage, resident_set);
	// std::cout << "TcpServer::checkMaxMemory " << vm_usage << " " << resident_set
		// << " " << _max_vm_kb << " " << _max_rss_kb << std::endl;
	if(vm_usage > _max_vm_kb || resident_set > _max_rss_kb) {
		_exceedMaxMemory = true;
		// std::cout << "TcpServer::checkMaxMemory _exceedMaxMemory = true" << std::endl;
	}
	else
		_exceedMaxMemory = false;
}

void TcpServer::updateTime()
{
	char timeBuffer[20];
	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);
    sprintf(timeBuffer, "%4d/%02d/%02d %02d:%02d:%02d",
                       timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
                       timeinfo->tm_mday, timeinfo->tm_hour,
                       timeinfo->tm_min, timeinfo->tm_sec);
    timeBuffer[19] = '\0';
    std::string temp(timeBuffer);
    _timeString.swap(temp);
}

/*
 *	starts all the worker threads and the acceptor
 * */
void TcpServer::start(const int & n)
{
	int maxConnection = std::min(_max_tcp_per_worker, _max_tcp / n);
	// for epoll, maxevents must be greater than 0
	if(maxConnection <= 0)
		maxConnection = 1;
	for(int i = 0; i < n; ++i) {
		/* FIXME 100 */
		_threadPool.push_back(new Worker(this));
		_threadPool[i]->setMaxConnection(maxConnection);
		_threadPool[i]->start();
	}
	_acceptor.start(10);
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

static int convertToKB(std::string & value)
{
	char c = value.back();
	int factor = 0;
	if(c == 'K')
		factor = 1;
	else if(c == 'M')
		factor = 1024;
	else if(c == 'G')
		factor = 1024 * 1024;
	value[value.size() - 1] = '\0';
	return factor * atoi(value.data());
}

void TcpServer::loadConfig(const char * configFileName)
{
	std::cout << "loading config : " << configFileName << std::endl;

	std::ifstream configFile(configFileName, std::ios::in);
	if(configFile.is_open())
	{
		std::string line = "";
		while(true) {
			if(configFile.eof())
				break;
			getline(configFile, line);

			if(line.empty())
				continue;
			if(line[0] == '#')
				continue;

			std::vector<std::string> currentLine = split(line, "=");
			if(currentLine.size() != 2)
				break;

			std::string key = trim(currentLine[0]);
			std::string value = trim(currentLine[1]);


			if(key == "listening-port")
			{
				_listenning_port = atoi(value.data());
				std::cout << "config : listening-port " << _listenning_port << std::endl;
			}
			else if(key == "max-tcp")
			{
				_max_tcp = atoi(value.data());
				std::cout << "config : max-tcp" << " " << _max_tcp << std::endl;
			}
			else if(key == "max-tcp-per-worker")
			{
				_max_tcp_per_worker = atoi(value.data());
				std::cout << "config : max-tcp-per-worker" << " " << _max_tcp_per_worker << std::endl;
			}
			else if(key == "max-tcp-eviction-rule")
			{
				if(value == "none")
					_max_tcp_eviction_rule = MAX_TCP_EVICTION_NONE;
				else if(value == "random")
					_max_tcp_eviction_rule = MAX_TCP_EVICTION_RANDOM;
				else if(value == "inactive")
					_max_tcp_eviction_rule = MAX_TCP_EVICTION_INACTIVE;
				std::cout << "config : max-tcp-eviction-rule" << " " << value << std::endl;
			}
			else if(key == "inactive-tcp-timeout")
			{
				_inactive_tcp_timeout = atoi(value.data());
				std::cout << "config : inactive-tcp-timeout" << " " << _inactive_tcp_timeout << std::endl;
			}
			else if(key == "inactive-tcp-eviction-rule")
			{
				if(value == "none")
					_inactive_tcp_eviction_rule = INACTIVE_TCP_EVICTION_NONE;
				else if(value == "close")
					_inactive_tcp_eviction_rule = INACTIVE_TCP_EVICTION_CLOSE;
				std::cout << "config : inactive-tcp-eviction-rule" << " " << value << std::endl;
			}

			else if(key == "max-vm-memory")
			{
				_max_vm_kb = convertToKB(value);
				std::cout << "config : max-vm-memory " << _max_vm_kb << std::endl;
			}
			else if(key == "max-rss")
			{
				_max_rss_kb = convertToKB(value);
				std::cout << "config : max-rss " << _max_rss_kb << std::endl;
			}

			else if(key == "eviction-pool-size")
			{
				_eviction_pool_size = atoi(value.data());
				std::cout << "config : eviction-pool-size" << " " << value << std::endl;
			}
			else if(key == "load-balance")
			{
				if(value == "round-robin")
					_loadBalance = std::bind(&TcpServer::loadBalanceRoundRobin, this, std::placeholders::_1);
					// _load_balance = LOAD_BALANCE_ROUNDROBIN;
				else if(value == "random")
					_loadBalance = std::bind(&TcpServer::loadBalanceRandom, this, std::placeholders::_1);
					// _load_balance = LOAD_BALANCE_RANDOM;
				else if(value == "min-connection")
					_loadBalance = std::bind(&TcpServer::loadBalanceMinConnection, this, std::placeholders::_1);
				else if(value == "ip-hash")
					_loadBalance = std::bind(&TcpServer::loadBalanceIPHash, this, std::placeholders::_1);
					// _load_balance = LOAD_BALANCE_MINCONNECTION;
				std::cout << "config : load-balance" << " " << value << std::endl;
			}
			else if(key == "tcp-read-buffer-init-size-bytes")
			{
				// _tcp_read_buffer_init_size_bytes = atoi(value.data());
				_tcp_read_buffer_init_size_bytes = 1024 * convertToKB(value);
				std::cout << "config : tcp-read-buffer-size-bytes" << " " << _tcp_read_buffer_init_size_bytes << std::endl;
			}
			else if(key == "tcp-write-buffer-init-size-bytes")
			{
				// _tcp_write_buffer_init_size_bytes = atoi(value.data());
				_tcp_write_buffer_init_size_bytes = 1024 * convertToKB(value);
				std::cout << "config : tcp-write-buffer-size-bytes" << " " << _tcp_write_buffer_init_size_bytes << std::endl;
			}
			else if(key == "tcp-read-buffer-max-size-bytes")
			{
				// _tcp_read_buffer_max_size_bytes = atoi(value.data());
				_tcp_read_buffer_max_size_bytes = 1024 * convertToKB(value);
				std::cout << "config : tcp-read-buffer-size-bytes" << " " << _tcp_read_buffer_max_size_bytes << std::endl;
			}
			else if(key == "tcp-write-buffer-max-size-bytes")
			{
				// _tcp_write_buffer_max_size_bytes = atoi(value.data());
				_tcp_write_buffer_max_size_bytes = 1024 * convertToKB(value);
				std::cout << "config : tcp-write-buffer-size-bytes" << " " << _tcp_write_buffer_max_size_bytes << std::endl;
			}
			else if(key == "log-file-name-prefix")
			{
				_log_file_name_prefix = value;
				std::cout << "config : log-file-name-prefix" << " " << _log_file_name_prefix << std::endl;
			}
			else if(key == "log-file-rolling")
			{
				if(value == "none")
					_log_file_rolling = LOG_FILE_ROLLING_NONE;
				else if(value == "size")
					_log_file_rolling = LOG_FILE_ROLLING_SIZE;
				else if(value == "daily")
					_log_file_rolling = LOG_FILE_ROLLING_DAILY;
				else if(value == "size & daily")
					_log_file_rolling = LOG_FILE_ROLLING_SIZEDAILY;
				std::cout << "config : log-file-rolling " << _log_file_rolling << std::endl;
			}
			else if(key == "log-file-max-size")
			{
				// _log_file_max_size_bytes = atoi(value.data());
				_log_file_max_size_bytes = 1024 * convertToKB(value);
				std::cout << "config : log-file-max-size" << " " << _log_file_max_size_bytes << std::endl;
			}
			else if(key == "log-flush-interval-second")
			{
				_log_flush_interval_second = atoi(value.data());
				std::cout << "config : log-flush-interval-second" << " " << _log_flush_interval_second << std::endl;
			}
			else if(key == "log-high-watermask")
			{
				_log_high_watermask = atoi(value.data());
				std::cout << "config : log-high-watermask" << " " << _log_high_watermask << std::endl;
			}
		}
	}
	std::cout << "loading config done..." << std::endl;
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

int TcpServer::getEvictionPoolSize() const {
	return _eviction_pool_size;
}

void TcpServer::log(const char * level, pthread_t threadID,
					const std::string & threadName,
					uint32_t ip, uint16_t port, const std::string & msg)
{
	_logger.append(format("%s %u %s %s %d.%d.%d.%d:%d %s",
					level, threadID, threadName.data(),
					_timeString.data(),
					(ip & 0x000000FF),
					(ip & 0x0000FF00) >> 8,
					(ip & 0x00FF0000) >> 16,
					(ip & 0xFF000000) >> 24,
					port, msg.data()));
}

bool TcpServer::exceedMaxMemory() const
{
	return _exceedMaxMemory;
}