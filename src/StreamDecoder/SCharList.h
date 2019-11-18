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
