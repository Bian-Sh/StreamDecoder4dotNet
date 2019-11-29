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

	//���ܳɹ���� ���ͷ�frame�ռ�
	//void Repaint(struct AVFrame *frame);

	//����YUV����
	bool Repaint(unsigned char* yuv[]);
	bool Repaint(Frame* frame);

	void Close();

	bool isValid = false;
protected:
	//ˢ����ʾ
	void paintGL();

	//��ʼ��GL
	void initializeGL();

	//���ڳߴ�仯
	//void resizeGL(int w, int h);

	void closeEvent(QCloseEvent *event);

private:
	QMutex mux;
	//shader����
	QGLShaderProgram program;
	//��shader��ȡ�е�yuv������ַ
	GLuint unis[3] = { 0 };
	//opengltexture��ַ
	GLuint texs[3] = { 0 };

	int width = 1920;
	int height = 1080;

	//YUV���� ��Ҫ�ͷ�
	unsigned char* datas[3] = { 0 };

	bool isRepainting = false;

	bool isExit = false;

	//Session* session = NULL;
};

