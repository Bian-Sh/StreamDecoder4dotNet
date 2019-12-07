#pragma once

#include "Canvas.h"
#include <QWidget>

class CanvasRGBA : public Canvas
{
	Q_OBJECT

public:
	CanvasRGBA(QWidget *parent = Q_NULLPTR);
	~CanvasRGBA();

	virtual void Init(int width, int height);

	//����YUV����
	virtual bool Repaint(Frame* frame);

protected:

	//��ʼ��GL
	void initializeGL();

	//ˢ����ʾ
	void paintGL();

	//���ڳߴ�仯
	//void resizeGL(int w, int h);

private:

	
	//��shader��ȡ�е�yuv������ַ
	GLuint uniform;
	//opengltexture��ַ
	GLuint tex;

	unsigned char* rgba = NULL;

};
