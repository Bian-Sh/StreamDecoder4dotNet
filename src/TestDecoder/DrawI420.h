#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QGLShaderProgram>
#include <QMutex>
#include <QWaitCondition>
struct Frame;
class DrawI420 : public QOpenGLWidget, protected QOpenGLFunctions
{

	Q_OBJECT

public:
	DrawI420();
	~DrawI420();

	void Init(int width, int height);

	//不管成功与否， 都释放frame空间
	//void Repaint(struct AVFrame *frame);

	//绘制YUV数据
	bool Repaint(unsigned char* yuv[]);
	bool Repaint(Frame* frame);

	void Close();

	bool isValid = false;
protected:
	//刷新显示
	void paintGL();

	//初始化GL
	void initializeGL();

	//窗口尺寸变化
	//void resizeGL(int w, int h);

	void closeEvent(QCloseEvent *event);

private:
	QMutex mux;
	//shader程序
	QGLShaderProgram program;
	//从shader获取中的yuv变量地址
	GLuint unis[3] = { 0 };
	//opengltexture地址
	GLuint texs[3] = { 0 };

	int width = 1920;
	int height = 1080;

	//YUV数据 需要释放
	unsigned char* datas[3] = { 0 };

	bool isRepainting = false;

	bool isExit = false;

	//Session* session = NULL;
};

