#include "CanvasI420.h"

#include <QDebug>
#include <QThread>
#include "Session.h"
#include <QCoreApplication>
#include "StreamDecoder.h"
#include "Packet.h"
#define GET_STR(x) #x
#define A_VER 3
#define T_VER 4

//����shader
char*vString = GET_STR(
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
char* tString = GET_STR(
	varying vec2 textureOut;
uniform sampler2D tex_y;
uniform sampler2D tex_u;
uniform sampler2D tex_v;
void main(void)
{
	vec3 yuv;
	vec3 rgb;
	yuv.x = texture2D(tex_y, textureOut).r;
	yuv.y = texture2D(tex_u, textureOut).r - 0.5;
	yuv.z = texture2D(tex_v, textureOut).r - 0.5;
	rgb = mat3(1.0, 1.0, 1.0,
		0, -0.39465, 2.03211,
		1.13983, -0.58060, 0.0
	)* yuv;
	//����Ҷ�
	//float gray = rgb.r * 0.299 + rgb.g * 0.587 + rgb.b * 0.114;
	//gl_FragColor = vec4(gray, gray, gray, 1.0);
	gl_FragColor = vec4(rgb, 1.0);
}
);

CanvasI420::CanvasI420(QWidget *parent)
	: Canvas(parent)
{
}


CanvasI420::~CanvasI420()
{
	mux.lock();
	delete datas[0];
	delete datas[1];
	delete datas[2];
	datas[0] = NULL;
	mux.unlock();
}

void CanvasI420::Init(int width, int height)
{
	QMutexLocker locker(&mux);
	this->width = width;
	this->height = height;

	delete datas[0];
	delete datas[1];
	delete datas[2];
	//�����ڴ�ռ�
	datas[0] = new unsigned char[width * height * 2];
	datas[1] = new unsigned char[width * height / 2];
	datas[2] = new unsigned char[width * height / 2];

	//����
	if (texs[0])
	{
		glDeleteTextures(3, texs);
	}
	//��������
	glGenTextures(3, texs);
	//Y
	glBindTexture(GL_TEXTURE_2D, texs[0]);
	//�Ŵ���С�����������Բ�ֵ  GL_NEAREST(Ч�ʸߣ�������������)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//�������ʿռ�(�Կ��ռ䣩
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);

	//U
	glBindTexture(GL_TEXTURE_2D, texs[1]);
	//�Ŵ���С�����������Բ�ֵ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//�������ʿռ�(�Կ��ռ䣩
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width / 2, height / 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);

	//V
	glBindTexture(GL_TEXTURE_2D, texs[2]);
	//�Ŵ���С�����������Բ�ֵ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//�������ʿռ�(�Կ��ռ䣩
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width / 2, height / 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);

}

void CanvasI420::initializeGL()
{
	QMutexLocker locker(&mux);
	initializeOpenGLFunctions();

	//����shader�ű�
	program.addShaderFromSourceCode(QGLShader::Vertex, vString);
	program.addShaderFromSourceCode(QGLShader::Fragment, tString);
	//���ö�������ı���
	program.bindAttributeLocation("vertexIn", A_VER);
	//���ò�������
	program.bindAttributeLocation("textureIn", T_VER);

	program.link();
	program.bind();

	//���ݶ���Ͳ�������
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

	//��shader��ȡ����
	unis[0] = program.uniformLocation("tex_y");
	unis[1] = program.uniformLocation("tex_u");
	unis[2] = program.uniformLocation("tex_v");

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}


//void DrawI420::Repaint(struct AVFrame *frame)
//{
//	while (isRepainting) {
//		if (isExit) return;
//		QThread::msleep(1);
//	}
//	qDebug() << "Repaint";
//	isRepainting = true;
//	if (!frame) return;
//	QMutexLocker locker(&mutex);
//	//��֤�ߴ���ȷ����֤����Ƶ֡
//	if (!datas[0] || width * height == 0 || frame->width != this->width || frame->height != this->height)
//	{
//		av_frame_free(&frame);
//		return;
//	}
//	//����Ҫ����
//	if (width == frame->linesize[0])
//	{
//		memcpy(datas[0], frame->data[0], width * height);
//		memcpy(datas[1], frame->data[1], width * height / 4);
//		memcpy(datas[2], frame->data[2], width * height / 4);
//	}
//	else
//	{
//		//�ж���
//		for (int i = 0; i < height; i++)
//		{
//			memcpy(datas[0] + width * i, frame->data[0] + frame->linesize[0] * i, width);
//		}
//		for (int i = 0; i < height / 2; i++)
//		{
//			memcpy(datas[1] + width / 2 * i, frame->data[1] + frame->linesize[1] * i, width);
//		}
//		for (int i = 0; i < height / 2; i++)
//		{
//			memcpy(datas[2] + width / 2 * i, frame->data[2] + frame->linesize[2] * i, width);
//		}
//	}
//
//	locker.unlock();
//	av_frame_free(&frame);
//	//ˢ����ʾ
//	update();
//}

//bool CanvasI420::Repaint(unsigned char* yuv[])
//{
//
//	if (isRepainting) return false;
//	
//	isRepainting = true;
//	memcpy(datas[0], yuv[0], width * height);
//	memcpy(datas[1], yuv[1], width * height / 4);
//	memcpy(datas[2], yuv[2], width * height / 4);
//	//QThread::msleep(30);
//	update();
//
//	return true;
//}

bool CanvasI420::Repaint(Frame* frame)
{
	if (isRepainting) return false;

	isRepainting = true;
	if (!datas[0])
	{
		return false;
	}
	memcpy(datas[0], frame->frame_y, frame->width * frame->height);
	memcpy(datas[1], frame->frame_u, frame->width * frame->height / 4);
	memcpy(datas[2], frame->frame_v, frame->width * frame->height / 4);
	//QThread::msleep(30);
	update();

	return true;
}

//void CanvasI420::Close()
//{
//	if (isExit)return;
//	isExit = true;
//	QMutexLocker locker(&mux);
//
//	delete datas[0];
//	delete datas[1];
//	delete datas[2];
//	datas[0] = NULL;
//	datas[1] = NULL;
//	datas[2] = NULL;
//
//	//����
//	if (texs[0])
//	{
//		glDeleteTextures(3, texs);
//	}
//
//	close();
//}

void CanvasI420::paintGL()
{
	if (isExit) return;
	QMutexLocker locker(&mux);

	glActiveTexture(GL_TEXTURE0);
	//�󶨵�����
	glBindTexture(GL_TEXTURE_2D, texs[0]);
	//glPixelStorei(GL_UNPACK_ROW_LENGTH, linesize[0]);
	//�޸Ĳ�������
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE, datas[0]);
	//��shader��������
	glUniform1i(unis[0], 0);

	glActiveTexture(GL_TEXTURE1);
	//�󶨵�����
	glBindTexture(GL_TEXTURE_2D, texs[1]);
	//glPixelStorei(GL_UNPACK_ROW_LENGTH, linesize[1]);
	//�޸Ĳ�������
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_LUMINANCE, GL_UNSIGNED_BYTE, datas[1]);
	//��shader��������
	glUniform1i(unis[1], 1);

	glActiveTexture(GL_TEXTURE2);
	//�󶨵�����
	glBindTexture(GL_TEXTURE_2D, texs[2]);
	//glPixelStorei(GL_UNPACK_ROW_LENGTH, linesize[2]);
	//�޸Ĳ�������
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_LUMINANCE, GL_UNSIGNED_BYTE, datas[2]);
	//��shader��������
	glUniform1i(unis[2], 2);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	isRepainting = false;
}
