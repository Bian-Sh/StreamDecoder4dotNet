#pragma once
typedef struct RGB32 {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
	unsigned char alpha;
} RGB32;

void InitConverter();
void I420toRGB(const unsigned char* ybuf, const unsigned char* ubuf, const unsigned char* vbuf, int width, int height, const unsigned char* rgb);

