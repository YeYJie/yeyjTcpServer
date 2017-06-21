#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "include.h"

namespace yeyj
{


class Buffer
{

public:

	Buffer(int initSize, int maxSize, float growFactor = 1.5)
	{
		assert(initSize <= maxSize);
		_currentSize = initSize;
		_maxSize = maxSize;
		_begin = 0;
		_end = 0;
		_count = 0;
		_growFactor = growFactor;
		_data = new char[_currentSize]{0};
		assert(_data);
	}

	~Buffer()
	{
		delete[] _data;
		_begin = _end = _count = _currentSize = _maxSize = _growFactor = 0;
	}

	int size() const { return _count; }

	bool empty() const { return _count == 0; }

	bool isFull() const { return _count == _currentSize; }

	bool canGrow() const { return _currentSize < _maxSize; }

	bool needGrow(int n) const { return _count + n > _currentSize; }

	void clear() {
		_begin = 0;
		_end = 0;
		_count = 0;
	}

	std::string readAsString() {
		int count = _count;
		// char buffer[count + 1];
		char * buffer = new char[count + 1];
		read(buffer);
		buffer[count] = '\0';
		std::string res(buffer);
		delete[] buffer;
		return res;
		// return std::string(buffer);
	}

	std::string readAsString(int length) {
		int count = length > _count ? _count : length;
		char * buffer = new char[count + 1];
		read(buffer, count);
		buffer[count] = '\0';
		std::string res(buffer);
		delete[] buffer;
		return res;
	}

	int read(char * dst) {
		return read(dst, _count);
	}

	int read(char * dst, int n) {
		n = n > _count ? _count : n;
		for(int i = 0; i < n; ++i) {
			dst[i] = _data[_begin];
			_begin = (_begin + 1) % _currentSize;
			--_count;
		}
		return n;
	}

	void skipHead(int n) {
		assert(_count >= n);
		_count -= n;
		_begin = (_begin + n) % _currentSize;
	}

	void skipTail(int n) {
		assert(_count >= n);
		_count -= n;
		_end = (_end - n) % _currentSize;
	}

	int copy(char * dst) const {
		return copy(dst, _count);
	}

	int copy(char * dst, int n) const {
		n = n > _count ? _count : n;
		int begin = _begin;
		for(int i = 0; i < n; ++i) {
			dst[i] = _data[begin];
			begin = (begin + 1) % _currentSize;
		}
		return n;
	}

	int getSpace() const { return _currentSize - _count; }

	int getMaxSpace() const { return _maxSize - _count; }

	int getSize() const { return _count; }

	int write(const std::string & src) {
		return write(src.data(), src.size());
	}

	int write(const char * src) {
		return write(src, strlen(src));
	}

	int write(const char * src, int n) {
		if(needGrow(n) && canGrow())
			grow(_count + n);
		int room = getSpace();
		n = n > room ? room : n;
		for(int i = 0; i < n; ++i) {
			_data[_end] = src[i];
			_end = (_end + 1) % _currentSize;
			++_count;
		}
		return n;
	}

	// check nothing and never cause the buffer to grow
	void putChar(const char c)
	{
		_data[_end] = c;
		_end = (_end + 1) % _currentSize;
		++_count;
	}

	// check nothing
	char getChar()
	{
		char res = _data[_begin];
		_begin = (_begin + 1) % _currentSize;
		--_count;
	}

	// chech nothing
	char peekChar(int offset) const
	{
		int index = (_begin + offset) % _currentSize;
		return _data[index];
	}

	// return 1 if success, 0 if fails
	int peekIntWithoutLength(int * res, const char delimiter = ' ') const
	{
		int offset = 0;
		*res = 0;
		char currentChar;
		while(offset + 1 <= _count)
		{
			currentChar = peekChar(offset);
			if(isdigit(currentChar))
				*res = *res * 10 + (currentChar - '0');
			else
				break;
			++offset;
		}
		if(currentChar != delimiter)
			return 0;
		else
			return 1;
	}

	int peekIntWithLength(int * res, int * length, const char delimiter = ' ') const
	{
		int offset = 0;
		*res = 0;
		char currentChar;
		while(offset + 1 <= _count)
		{
			currentChar = peekChar(offset);
			if(isdigit(currentChar))
				*res = *res * 10 + (currentChar - '0');
			else
				break;
			++offset;
		}
		*length = offset;
		if(currentChar != delimiter)
			return 0;
		else
			return 1;
	}

	void print() {
		printf("print buffer[%d][%d][%d][%d][", _currentSize,
					_begin, _end, _count);
		for(int i = 0; i < _currentSize; ++i)
			printf("%c", _data[i]);
		cout << "]" << endl;
	}

	void grow(int capacity) {
		int newSize = _currentSize;
		while(newSize < capacity)
			newSize *= _growFactor;
		newSize = newSize > _maxSize ? _maxSize : newSize;

		char * temp = new char[newSize]{0};

		int count = _count;
		read(temp, count);
		_count = count;
		_begin = 0;
		_end = _count;
		_currentSize = newSize;
		swap(temp, _data);
		delete[] temp;
	}

private:

	char * 		_data;

	int 		_currentSize;
	int 		_maxSize;

	int 		_begin;
	int 		_end;
	int 		_count;

	float		_growFactor;
};


}

#endif