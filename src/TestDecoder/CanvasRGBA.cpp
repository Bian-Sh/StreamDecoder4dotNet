#include "CanvasRGBA.h"
#include <QDebug>
#include <QMutexLocker>
#include "StreamDecoder.h"
#include "Packet.h"

#define GET_STR(x) #x
#define A_VER 0
#define T_VER 1

//����shader
char* rgb_vertex_Code = GET_STR(
	attribute vec4 vertexIn;
attribute vec2 textureIn;
varying vec2 textureOut;

void main(void)
{
	gl_Position = vertexIn;
	textureOut = textureIn;
}
);

//ƬԪshader
char* rgb_frag_Code = GET_STR(
	varying vec2 textureOut;
uniform sampler2D tex;

void main(void)
{
	gl_FragColor = texture2D(tex, textureOut);
}
);

CanvasRGBA::CanvasRGBA(QWidget *parent)
	: Canvas(parent)
{
}

CanvasRGBA::~CanvasRGBA()
{
	isExit = true;
	QMutexLocker locker(&mux);
	if (rgba)
	{
		delete rgba;
		rgba = NULL;
	}
}

void CanvasRGBA::Init(int width, int height)
{
	QMutexLocker locker(&mux);
	this->width = width;
	this->height = height;

	if (rgba)
	{
		delete rgba;
		rgba = NULL;
	}

	rgba = new unsigned char[width * height * 4];

	if (tex) glDeleteTextures(1, &tex);

	//��������
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	//�Ŵ���С�����������Բ�ֵ  GL_NEAREST(Ч�ʸߣ�������������)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//�������ʿռ�(�Կ��ռ䣩
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
}


bool CanvasRGBA::Repaint(Frame* frame)
{
	if (isRepainting || !rgba) return false;
	isRepainting = true;
	if (!frame->rgba)
	{
		qDebug() << "rgba is null";
		return false;
	}
	memcpy(rgba, frame->rgba, width * height * 4);
	update();
	return true;
}

void CanvasRGBA::initializeGL()
{
	initializeOpenGLFunctions();
	
	//����Shader
	program.addShaderFromSourceCode(QGLShader::Vertex, rgb_vertex_Code);
	program.addShaderFromSourceCode(QGLShader::Fragment, rgb_frag_Code);

	//���ö�������ı���
	program.bindAttributeLocation("vertexIn", A_VER);
	//���ò�������
	program.bindAttributeLocation("textureIn", T_VER);

	program.link();
	program.bind();

	//����
	static const GLfloat ver[] = {
		-1.0f,	-1.0f,
		1.0f,	-1.0f,
		-1.0f,	1.0f,
		1.0f,	1.0f
	};
	//��������
	static const GLfloat tex[] = {
		0.0f,	1.0f,
		1.0f,	1.0f,
		0.0f,	0.0f,
		1.0f,	0.0f
	};

	//����
	glVertexAttribPointer(A_VER, 2, GL_FLOAT, false, 0, ver);
	glEnableVertexAttribArray(A_VER);

	//����
	glVertexAttribPointer(T_VER, 2, GL_FLOAT, false, 0, tex);
	glEnableVertexAttribArray(T_VER);

	uniform = program.uniformLocation("tex");

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

}

void CanvasRGBA::paintGL()
{
	if (isExit) return;

	QMutexLocker locker(&mux);
	glActiveTexture(GL_TEXTURE0);
	//�󶨵�����
	glBindTexture(GL_TEXTURE_2D, tex);
	//glPixelStorei(GL_UNPACK_ROW_LENGTH, linesize[0]);
	//�޸Ĳ�������
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
	//��shader��������
	glUniform1i(uniform, 0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	isRepainting = false;
}

