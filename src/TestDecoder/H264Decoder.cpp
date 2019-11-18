#include "H264Decoder.h"
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QMutexLocker>
#include "StreamDecoder.h"
#include <QCryptographicHash>
#include <Session.h>
#include "Packet.h"
#include "DrawI420.h"
#pragma comment(lib, "StreamDecoder.lib")

H264Decoder* H264Decoder::self = NULL;

void H264Decoder::OnDrawFrame(DotNetFrame* frame)
{
	if (canvas == NULL)
	{
		canvas = new DrawI420();
		
		canvas->resize(frame->width, frame->height);
		canvas->show();
		canvas->Init(frame->width, frame->height);
	}
	canvas->Repaint(frame);
}

H264Decoder::H264Decoder(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	if (self == NULL) self = this;
	StreamDecoderInitialize(NULL, &OnDraw);
}


void OnDraw(DotNetFrame* frame)
{
	H264Decoder::self->OnDrawFrame(frame);
}

void H264Decoder::on_createsession_clicked()
{
	if (session) return;
	
	isExit = false;
	session = CreateSession();
}

void H264Decoder::on_deletesession_clicked()
{
	isExit = true;
	if (!session) return;
	DeleteSession(session);
	session = NULL;
}

void H264Decoder::on_OpenDemux_clicked()
{
	if (!session) return;
	qDebug() << OpenDemuxThread(session, 2000);
	if (!isRunthread)
	{
		isExit = false;
		QtConcurrent::run(this, &H264Decoder::mrun);
	}
		
}

void H264Decoder::on_begindecode_clicked()
{
	if (!session) return;
	BeginDecode(session);
}

void H264Decoder::on_stopdecode_clicked()
{
	if (!session) return;
	StopDecode(session);
	isExit = true;
}

void H264Decoder::on_GetFree_clicked()
{
	if (!session) return;
	qDebug() << GetCacheFreeSize(session);
}

void H264Decoder::closeEvent(QCloseEvent *event)
{
	on_deletesession_clicked();
}

void H264Decoder::mrun()
{
	isRunthread = true;
	qDebug() << "read stream thread start";
	
	if(!fp)
		fp = fopen("f:/HTTPServer/mv.mp4", "rb");
		
	if (!fp)
	{
		qDebug() << "open file failed";
		return;
	}

	if (!readBuff)
		readBuff = new char[1024];

	//QCryptographicHash hash(QCryptographicHash::Md5);
	int s = 0;
	while (!isExit)
	{
		if (!session || !fp || feof(fp)) break;
		
		int len = fread(readBuff, 1, 1024, fp);
		if (len <= 0)
		{
			qDebug() << "read ok";
			break;
		}
		while (session && !isExit)
		{
			if (PushStream2Cache(session, readBuff, len))
				break;

			QThread::msleep(1);
			continue;
		}
		//hash.addData(readBuff, len);
		QThread::msleep(1);
	}
	//qDebug() << "result:" << hash.result().toHex();
	fclose(fp);
	fp = NULL;
	isRunthread = false;
	qDebug() << "read stream thread quit";
}

