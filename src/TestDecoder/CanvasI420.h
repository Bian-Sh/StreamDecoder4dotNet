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

	//绘制YUV数据
	virtual void Repaint(Frame* frame);

protected:
	//初始化GL
	void initializeGL();

	//刷新显示
	void paintGL();

	//窗口尺寸变化
	//void resizeGL(int w, int h);

private:

	//从shader获取中的yuv变量地址
	GLuint unis[3] = { 0 };
	//opengltexture地址
	GLuint texs[3] = { 0 };


	//YUV数据 需要释放
	unsigned char* datas[3] = { 0 };
};

