#pragma once
typedef struct RGB32 {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
	unsigned char alpha;
} RGB32;

void InitConverter();


//�ڴ��д洢��ʽΪRGBA(���ֽ�->���ֽ�)
void I420toRGBA(const unsigned char* ybuf, const unsigned char* ubuf, const unsigned char* vbuf, int width, int height, const unsigned char* rgb);

//�ڴ��д洢��ʽΪBGRA(���ֽ�->���ֽ�)
void I420toBGRA(const unsigned char* ybuf, const unsigned char* ubuf, const unsigned char* vbuf, int width, int height, const unsigned char* rgb);

