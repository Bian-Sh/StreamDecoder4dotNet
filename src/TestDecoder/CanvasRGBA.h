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

	//绘制YUV数据
	virtual bool Repaint(Frame* frame);

protected:

	//初始化GL
	void initializeGL();

	//刷新显示
	void paintGL();

	//窗口尺寸变化
	//void resizeGL(int w, int h);

private:

	
	//从shader获取中的yuv变量地址
	GLuint uniform;
	//opengltexture地址
	GLuint tex;

	unsigned char* rgba = NULL;

};
