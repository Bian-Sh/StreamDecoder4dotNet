#include "H264Decoder.h"
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QMutexLocker>
#include "StreamDecoder.h"
#include <QCryptographicHash>
#include <Session.h>
#include "Packet.h"
#include "DrawI420.h"
#include <QFileDialog>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#pragma comment(lib, "StreamDecoder.lib")

#define USE_WIDGET_

H264Decoder* H264Decoder::self = NULL;

void H264Decoder::OnDrawFrame(DotNetFrame* frame)
{
#ifdef USE_WIDGET
	if (width != frame->width || height != frame->height)
	{
		width = frame->width;
		height = frame->height;
		canvas->resize(width, height);

		canvas->Init(frame->width, frame->height);
	}
	if (canvas)
		canvas->Repaint(frame);
#endif

}

void H264Decoder::OnEventPkt(int playerID, int eventType)
{
	//BeginDecode(session);
}


H264Decoder::H264Decoder(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.filePath->setText(filePath);
	if (self == NULL) self = this;
	StreamDecoderInitialize(NULL, &OnDraw, &OnEvent);

#ifdef USE_WIDGET
	if (canvas == NULL)
	{
		canvas = new DrawI420();
		canvas->show();
	}
#endif

	mServer = new QTcpServer(this);

	connect(mServer, &QTcpServer::newConnection, [this]() {
		mSocket = mServer->nextPendingConnection();
		QString deviceName;
		QSize size;
		if (GetDeviceInfoOnConnect(deviceName, size))
		{
			qDebug() << deviceName;
			qDebug() << size;
		}
		//connect(mSocket, &QTcpSocket::readyRead, this, &H264Decoder::OnRead);
	});

	/*if (mServer->listen(QHostAddress("127.0.0.1"), 5555))
	{
		qDebug() << "Server start success";
	}*/


	QTimer *timer = new QTimer(parent);
	timer->setSingleShot(false);
	timer->setInterval(30);
	//timer->start();

	//on_createsession_clicked();
	connect(timer, &QTimer::timeout, [this]() {
		//on_trydemux_clicked();
		static bool b = true;
		if (b)
		{
			on_CreateSession_clicked();
			on_TryBitStreamDemux_clicked();
		}
		else
		{
			on_DeleteSession_clicked();
		}
		b = !b;
	});

}

void H264Decoder::OnRead()
{
	QByteArray msg = mSocket->readAll();
	qDebug() << msg;
}



void OnDraw(DotNetFrame* frame)
{
	H264Decoder::self->OnDrawFrame(frame);
}

void OnEvent(int playerID, int eventType)
{
	H264Decoder::self->OnEventPkt(playerID, eventType);
}

void H264Decoder::on_CreateSession_clicked()
{
	if (session) return;
	session = CreateSession(1, 1000000);
	SetOption(session, DemuxTimeout, 5000);
	SetOption(session, PushFrameInterval, 0);
	SetOption(session, WaitBitStreamTimeout, 1000);
	SetOption(session, AlwaysWaitBitStream, false);
	SetOption(session, AutoDecode, true);
}

void H264Decoder::on_DeleteSession_clicked()
{

	if (!session) return;
	DeleteSession(session);
	session = NULL;
}

void H264Decoder::on_TryBitStreamDemux_clicked()
{
	if (!session) return;
	TryBitStreamDemux(session);

}

void H264Decoder::on_TryNetStreamDemux_clicked()
{
	if (!session) return;
	//TryNetStreamDemux(session, "rtmp://192.168.30.135/live/test");
	//TryNetStreamDemux(session, "rtmp://202.69.69.180:443/webcast/bshdlive-pc");
	TryNetStreamDemux(session, "rtmp://58.200.131.2:1935/livetv/hunantv");
	//TryNetStreamDemux(session, "rtmp://192.168.0.104/live/test");
}

void H264Decoder::on_BeginDecode_clicked()
{
	if (!session) return;
	BeginDecode(session);
}

void H264Decoder::on_StopDecode_clicked()
{
	if (!session) return;
	StopDecode(session);

}

void H264Decoder::on_GetFree_clicked()
{
	if (!session) return;
	qDebug() << GetCacheFreeSize(session);
}

void H264Decoder::on_OpenFile_clicked()
{
	qDebug() << "open";
	filePath = QFileDialog::getOpenFileName(this);
	ui.filePath->setText(filePath);
}


void H264Decoder::on_StartSendData_clicked()
{
	if (isExit)
		QtConcurrent::run(this, &H264Decoder::mrun);
}

void H264Decoder::on_EndSendData_clicked()
{
	isExit = true;
}


void H264Decoder::closeEvent(QCloseEvent *event)
{
#ifdef USE_WIDGET
	if (canvas)
	{
		canvas->deleteLater();
		canvas = NULL;
	}
#endif // USE_WIDGET


	on_DeleteSession_clicked();
}

void H264Decoder::mrun()
{
	isExit = false;
	qDebug() << "read stream thread start";

	if (!fp)
		fp = fopen(filePath.toLatin1(), "rb");

	if (!fp)
	{
		qDebug() << "open file failed";
		return;
	}

	if (!readBuff)
		readBuff = new char[10240];

	//QCryptographicHash hash(QCryptographicHash::Md5);
	int s = 0;
	while (!isExit)
	{
		if (!session || !fp || feof(fp)) break;

		int len = fread(readBuff, 1, 10240, fp);
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
	isExit = true;
	qDebug() << "read stream thread quit";
}


bool H264Decoder::GetDeviceInfoOnConnect(QString &deviceName, QSize &size)
{
	// abk001----------0xaaaa 0xbbbb
	// 64字节设备名称   2字节宽 2字节高
	unsigned char buf[68];
	if (mSocket->bytesAvailable() <= (68))
	{
		//等待300毫秒
		mSocket->waitForReadyRead(300);
	}
	qint64 len = mSocket->read((char*)buf, sizeof(buf));
	if (len < 68)
	{
		qInfo("Could not retrieve device infomation");
		return false;
	}
	buf[63] = '\0';
	deviceName = (char*)buf;
	//0x0438
	//第64个是0x04  左移8位为0x0400
	size.setWidth((buf[64] << 8) | buf[64 + 1]);
	size.setHeight(buf[64 + 2] << 8 | buf[64 + 3]);
	return true;
}