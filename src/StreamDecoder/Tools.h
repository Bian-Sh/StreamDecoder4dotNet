#pragma once
#include <sys/timeb.h>
extern "C"
{
#include <libavutil/rational.h>
}


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

	//获取UTC时间戳ms 
	inline long long GetTimestamp()
	{
		struct timeb t;
		ftime(&t);
		return 1000 * t.time + t.millitm;
	}

	inline static double r2d(AVRational r)
	{
		return r.den == 0 ? 0 : (double)r.num / (double)r.den;
	}
private:
	Tools();
	char* logbuf = 0;
};

