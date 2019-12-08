#pragma once
#include "Canvas.h"
struct Frame;
class CanvasI420 : public Canvas
{

	Q_OBJECT

public:
	CanvasI420(QWidget *parent = Q_NULLPTR);
	~CanvasI420();

	virtual void Init(int width, int height);

	//����YUV����
	virtual void Repaint(Frame* frame);

protected:
	//��ʼ��GL
	void initializeGL();

	//ˢ����ʾ
	void paintGL();

	//���ڳߴ�仯
	//void resizeGL(int w, int h);

private:

	//��shader��ȡ�е�yuv������ַ
	GLuint unis[3] = { 0 };
	//opengltexture��ַ
	GLuint texs[3] = { 0 };


	//YUV���� ��Ҫ�ͷ�
	unsigned char* datas[3] = { 0 };
};

