#include "ConvertYUV.h"

static unsigned short R_Y[256], R_U[256], R_V[256];
static unsigned short G_Y[256], G_U[256], G_V[256];
static unsigned short B_Y[256], B_U[256], B_V[256];
void InitConverter()
{
	for (int i = 0; i < 256; i++)
	{
		R_V[i] = 1.4075 * (i - 128);
		G_U[i] = 0.3455 * (i - 128);
		G_V[i] = 0.7169 * (i - 128);
		B_U[i] = 1.779 * (i - 128);
	}
}
void I420toRGB(const unsigned char* ybuf, const unsigned char* ubuf, const unsigned char* vbuf, int width, int height, const unsigned char* rgb)
{
	RGB32* rgbBuff = (RGB32*)rgb;

	//int piNum = width * height;

	for (int y = 0; y < height; y++)
	{
		int columnOffsetPi_y = y * width; //y乘以width的值
		int columnOffsetPi_uv = y / 2 * width / 2;
		for (int x = 0; x < width; x++)
		{
			//从左上角横向开始，第几个像素索引
			int index = columnOffsetPi_y + x;

			RGB32 *rgbNode = &rgbBuff[index];

			unsigned char y = ybuf[index];
			unsigned char u = ubuf[columnOffsetPi_uv + x / 2];
			unsigned char v = vbuf[columnOffsetPi_uv + x / 2];

			rgbNode->red = y + R_V[v];
			rgbNode->green = y - G_U[u] - G_V[v];
			rgbNode->blue = y + B_U[u];
			rgbNode->alpha = 255;

			/*rgbNode->red = y + 1.402 * (v - 128);
			rgbNode->green = y - 0.34413 * (u - 128) - 0.71414*(v - 128);
			rgbNode->blue = y + 1.772*(u - 128);
			rgbNode->alpha = 255;*/
		}
	}
}
