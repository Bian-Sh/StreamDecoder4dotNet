#pragma once
typedef struct RGB32 {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
	unsigned char alpha;
} RGB32;

void InitConverter();


//内存中存储方式为RGBA(低字节->高字节)
void I420toRGBA(const unsigned char* ybuf, const unsigned char* ubuf, const unsigned char* vbuf, int width, int height, const unsigned char* rgb);

//内存中存储方式为BGRA(低字节->高字节)
void I420toBGRA(const unsigned char* ybuf, const unsigned char* ubuf, const unsigned char* vbuf, int width, int height, const unsigned char* rgb);

