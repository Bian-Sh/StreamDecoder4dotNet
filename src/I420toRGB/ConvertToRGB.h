#pragma once

typedef struct RGB32 {
	unsigned char blue; 
	unsigned char green;     
	unsigned char red; 
	unsigned char alpha; 
} RGB32;


class ConvertToRGB
{
public:
	ConvertToRGB();
	~ConvertToRGB();

	void I420toRGB(const unsigned char* ybuf, const unsigned char* ubuf, const unsigned char* vbuf, int width, int height, const unsigned char* rgb);
};

