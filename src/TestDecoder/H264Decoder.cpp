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
#include <QCoreApplication>
#include "QtEvent.h"
#pragma comment(lib, "StreamDecoder.lib")

#define USE_WIDGET

H264Decoder* H264Decoder::self = NULL;

void H264Decoder::OnFrame(Frame* frame)
{
#ifdef USE_WIDGET

	if (width != frame->width || height != frame->height)
	{
		width = frame->width;
		height = frame->height;
		QtEvent * _event = new QtEvent(QtEvent::Event1);
		QCoreApplication::postEvent(this, _event);
		isSettingCanvas = true;
		return;
	}

	if (!isSettingCanvas && canvas)
		canvas->Repaint(frame);
#endif

}

bool H264Decoder::event(QEvent* event)
{
	if (event->type() == QtEvent::Event1)
	{
		canvas->resize(width, height);
		canvas->Init(width, height);
		isSettingCanvas = false;
		return true;
	}
	return QWidget::event(event);
}

void H264Decoder::OnSessionEvent(int playerID, int eventType)
{
	//BeginDecode(session);
}


H264Decoder::H264Decoder(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.filePath->setText(filePath);
	if (self == NULL) self = this;
	StreamDecoderInitialize(NULL, OnEvent_cb, OnDraw_cb);

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
	timer->setInterval(2000);
	//timer->start();

	//on_createsession_clicked();
	connect(timer, &QTimer::timeout, [this]() {

		static int value = 0;
		value++;
		if (value / 3 == 1)
		{
			on_CreateSession_clicked();
		}
		else if (value / 3 == 2)
		{

			on_TryBitStreamDemux_clicked();
			on_StartSendData_clicked();
		}
		else
		{
			on_EndSendData_clicked();
			on_DeleteSession_clicked();
		}
	});

}

void H264Decoder::OnRead()
{
	QByteArray msg = mSocket->readAll();
	qDebug() << msg;
}



void OnDraw_cb(Frame* frame) { H264Decoder::self->OnFrame(frame); }

void OnEvent_cb(int playerID, int eventType) { H264Decoder::self->OnSessionEvent(playerID, eventType); }

void EventTest(int playerID, int eventType)
{
	qDebug() << playerID;
}
void DrawTest(Frame* frame)
{
	qDebug() << frame->width;
}
void H264Decoder::on_CreateSession_clicked()
{
	if (session) return;
	session = CreateSession(1);
	SetOption(session, DataCacheSize, 1000000);
	SetOption(session, DemuxTimeout, 5000);
	SetOption(session, PushFrameInterval, 0);
	SetOption(session, WaitBitStreamTimeout, 1000);
	SetOption(session, AlwaysWaitBitStream, false);
	SetOption(session, AutoDecode, true);
	SetOption(session, DecodeThreadCount, 4);
	SetOption(session, UseCPUConvertYUV, false);
	SetOption(session, AsyncUpdate, true);
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
	TryNetStreamDemux(session, "rtmp://192.168.30.135/live/test");
	//TryNetStreamDemux(session, "rtmp://202.69.69.180:443/webcast/bshdlive-pc");
	//TryNetStreamDemux(session, "rtmp://58.200.131.2:1935/livetv/hunantv");
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

