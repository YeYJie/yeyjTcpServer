all :
	g++ -pthread main.cpp TcpServer.cpp Acceptor.cpp AsyncLogging.cpp Thread.cpp Mutex.cpp Worker.cpp TcpConnection.cpp -std=c++11

clean :
	rm ./a.out