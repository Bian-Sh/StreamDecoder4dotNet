#pragma once
#include <iostream>
class SCharList
{
public:
	//避免直接复制，主要用做取值
	char* arr = NULL;
public:
	SCharList(int maxSize)
	{
		arr = new char[maxSize + 1];
		memset(arr, 0, maxSize + 1);
		this->maxSize = maxSize;
	}
	~SCharList() {
		std::cout << "~SCharList" << std::endl;
		Dispose();
	}
	inline void push_back(char* buff, int len)
	{
		if (!arr || len <= 0) return;
		if (_size + len > maxSize)
		{
			std::cout << "Not enough memory!" << std::endl;
			throw "Not enough memory!";
			return;
		}
		memcpy(arr + _size, buff, len);
		_size += len;
	}
	inline void pop_front(int len)
	{
		if (!arr || len <= 0) return;
		if (len > _size)
		{
			std::cout << "Not enough memory!" << std::endl;
			throw "Remove Count Error!";
			return;
		}
		memcpy(arr, arr + len, _size - len);
		_size -= len;
		//memset(arr + _size, 0, maxSize - _size);
	}
	void Clear()
	{
		if (!arr) return;
		memset(arr, 0, maxSize + 1);
		_size = 0;
	}
	void Dispose()
	{
		Clear();
		if (arr)
		{
			delete arr;
			arr = NULL;
		}
	}

	inline int size()
	{
		if (arr)
			return _size;
		else
		{
			std::cout << "arr is null" << std::endl;
			throw "arr is null";
		}
			
	}
private:
	int maxSize = 0;
	int _size = 0;
};

struct LogPacket
{
	LogLevel _level;
	char* _log = 0;
	LogPacket(LogLevel level, char* log)
	{
		_level = level;
		int size = strlen(log);
		_log = new char[size + 1];
		_log[size] = 0;
		memcpy(_log, log, size);
	}
	~LogPacket()
	{
		if (_log)
		{
			delete _log;
			_log = NULL;
		}
	}
};

struct Frame
{
	int width;
	int height;
	int size_y;
	int size_u;
	int size_v;
	char* frame_y;
	char* frame_u;
	char* frame_v;
	
	Frame(int width, int height, char* y, char* u, char* v)
	{
		this->width = width;
		this->height = height;
		this->size_y = width * height;
		this->size_u = size_y / 4;
		this->size_v = size_y / 4;
		frame_y = new char[size_y];
		frame_u = new char[size_u];
		frame_v = new char[size_v];
		memcpy(frame_y, y, size_y);
		memcpy(frame_u, u, size_u);
		memcpy(frame_v, v, size_v);
	}
	~Frame()
	{
		if (frame_y)
		{
			delete frame_y;
			frame_y = NULL;
		}
		if (frame_u)
		{
			delete frame_u;
			frame_u = NULL;
		}
		if (frame_v)
		{
			delete frame_v;
			frame_v = NULL;
		}
	}
};

struct DotNetFrame
{
	int width;
	int height;
	char* frame_y;
	char* frame_u;
	char* frame_v;
};
