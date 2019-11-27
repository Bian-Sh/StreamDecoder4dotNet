#include "ConvertToRGB.h"



ConvertToRGB::ConvertToRGB()
{
}


ConvertToRGB::~ConvertToRGB()
{
}

void ConvertToRGB::I420toRGB(const unsigned char* ybuf, const unsigned char* ubuf, const unsigned char* vbuf, int width, int height, const unsigned char* rgb)
{
	//unsigned char* yuvBuffer = (unsigned char*)yuvBuffer;
	RGB32* rgbBuff = (RGB32*)rgb;

	int piNum = width * height;
	
	for (int y = 0; y < height; y++)
	{
		int yoffset = y / 2;
		for (int x = 0; x < width; x++)
		{
			int xoffset = x / 2;
			
			unsigned char y_Value = ybuf[y * width + x];
		}
	}
}
