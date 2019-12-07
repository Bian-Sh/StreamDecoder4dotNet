#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMutex>
#include <QGLShaderProgram>

struct Frame;
class Canvas : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	Canvas(QWidget *parent = Q_NULLPTR);
	~Canvas();

	virtual void Init(int width, int height) = 0;

	//����YUV����
	virtual bool Repaint(Frame* frame) = 0;

	bool isExit = false;

protected:

	QMutex mux;

	//shader����
	QGLShaderProgram program;

	int width = 0;
	int height = 0;
	bool isRepainting = false;
	
};
