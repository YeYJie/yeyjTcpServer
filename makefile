CXX = g++

CXXFLAGS = -g -std=c++11

LDFLAGS = -pthread


OBJS = main.o TcpServer.o Acceptor.o AsyncLogging.o Thread.o \
		Mutex.o Worker.o TcpConnection.o utilities.o

EXE = fyyj

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all : $(OBJS)
	$(CXX) -o $(EXE) $(OBJS) $(CXXFLAGS) $(LDFLAGS)

# test : test.o
# 	$(CXX) -O0 -o test test.o $(CXXFLAGS) $(LDFLAGS)

testBuffer : testBuffer.o buffer.h
	$(CXX) -O0 -o test testBuffer.o $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f *.o $(EXE)
