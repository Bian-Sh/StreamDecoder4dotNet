#pragma once
class Tools
{
public:
	
	inline static Tools* Get()
	{
		static Tools t;
		return &t;
	}

	char* av_strerror2(int errnum);

	void Sleep(int ms);
	~Tools();

private:
	Tools();
	char* logbuf = 0;
};

