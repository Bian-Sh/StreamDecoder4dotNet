#include "PlayerController.h"
#include <QDebug>
#include <QFileDialog>
#include <QtConcurrent/QtConcurrent>
#include "StreamDecoder.h"
#include "Packet.h"
#include "CanvasI420.h"
#include "QtEvent.h"
#include <QDesktopWidget>
#include "CanvasRGBA.h"

#define USE_WIDGET_
#define USE_RGBA_

PlayerController::PlayerController(QWidget *parent)
	: QWidget(parent), value(0x1122334455667788)
{
	ui.setupUi(this);
	ui.FilePath->setText("D:/HTTPServer/Faded.mp4");

#ifdef USE_WIDGET
#ifdef USE_RGBA
	canvas = new CanvasRGBA();
#else
	canvas = new CanvasI420();
#endif // USE_RGBA
	canvas->show();
#endif

	//测试效率定时器
	QTimer *timer = new QTimer(parent);
	timer->setSingleShot(false);
	timer->setInterval(10);
	//timer->start();

	connect(timer, &QTimer::timeout, [this]() {
		on_CreateSession_clicked();
		on_DeleteSession_clicked();
	});
}

PlayerController::~PlayerController()
{
	value = 0;
	isExit = true;
	while (isInSendThread)
	{
		QThread::msleep(1);
	}
#ifdef USE_WIDGET
	canvasMux.lock();
	if (canvas)
	{
		canvas->deleteLater();
		canvas = NULL;
	}
	canvasMux.unlock();
#endif // USE_WIDGET
	qDebug() << "~PlayerController";
}

bool PlayerController::isVaild()
{
	if (value == 0x1122334455667788)return true;
	else return false;
}



void PlayerController::closeEvent(QCloseEvent *event)
{

	this->deleteLater();
}

void PlayerController::on_OpenFile_clicked()
{
	QString filePath = QFileDialog::getOpenFileName(this, "select file", "F:/HTTPServer");
	ui.FilePath->setText(filePath);
}

void drawframe_callback(void* opaque, Frame* frame)
{
	PlayerController* pc = (PlayerController*)opaque;
	if (pc) pc->OnDrawFrameCb(frame);
}
void sessionevent_callback(void* opaque, int eventType)
{
	PlayerController* pc = (PlayerController*)opaque;
	if (pc) pc->OnSessionEventCb(eventType);
}

void PlayerController::OnDrawFrameCb(Frame * frame)
{
	if (!isVaild()) return;
#ifdef USE_WIDGET
	if (width != frame->width || height != frame->height)
	{
		width = frame->width;
		height = frame->height;

		isInSettingCanvas = true;
		QtEvent * _event = new QtEvent(QtEvent::SetCanvas);
		QCoreApplication::postEvent(this, _event);
	}

	if (isInSettingCanvas) return;
	canvasMux.lock();
	if (canvas)
	{
		

#ifdef USE_RGBA
		
		while (!isExit && frame->rgba)
		{
			if (canvas->Repaint(frame))
			{
				break;
			}
			QThread::msleep(1);
		}
#else
		while (!isExit)
		{
			if (canvas->Repaint(frame))
			{
				break;
			}
			QThread::msleep(1);
		}
#endif // USE_RGBA

	}
		
	canvasMux.unlock();
#endif
}

void PlayerController::OnSessionEventCb(int eventType)
{
	if (eventType == SessionEventType::DemuxSuccess)
	{
		qDebug() << "DemuxSuccess";
	}
}

//创建一个Session
void PlayerController::on_CreateSession_clicked()
{
	if (player) return;
	player = CreateSession();

	SetOption(player, OptionType::DataCacheSize, 1000000);
	SetOption(player, OptionType::DemuxTimeout, 2000);
	SetOption(player, OptionType::PushFrameInterval, 0);
	SetOption(player, OptionType::AlwaysWaitBitStream, false);
	SetOption(player, OptionType::WaitBitStreamTimeout, 1000);
	SetOption(player, OptionType::AutoDecode, false);
	SetOption(player, OptionType::DecodeThreadCount, 4);
	SetOption(player, OptionType::UseCPUConvertYUV, false);
	SetOption(player, OptionType::ConvertPixelFormat, PixelFormat::RGBA);
	SetOption(player, OptionType::AsyncUpdate, true);

	SetEventCallBack(player, sessionevent_callback, drawframe_callback, this);
	
}
//删除一个Session
void PlayerController::on_DeleteSession_clicked()
{
	if (!player) return;
	isInSendThread = false;
	DeleteSession(player);
	player = NULL;
}

void PlayerController::on_GetCacheFree_clicked()
{
	qDebug() << GetCacheFreeSize(player);
}

void PlayerController::on_TryBitStreamDemux_clicked()
{
	TryBitStreamDemux(player);
}

void PlayerController::on_TryNetStreamDemux_clicked()
{
	TryNetStreamDemux(player, "rtmp://192.168.30.166/live/test");
}

void PlayerController::on_BeginDecode_clicked()
{
	BeginDecode(player);
}

void PlayerController::on_EndDecode_clicked()
{
	isInSendThread = false;
	EndDecode(player);
}

void PlayerController::on_StartSendData_clicked()
{
	//当Player正在处于解码等待数据的时候，重开读取数据线程发送到dataCache，解码就会出错
	//视频流就发生了错误，是外在因素决定
	if (isInSendThread || isExit) return;
	QtConcurrent::run(this, &PlayerController::run);
}

void PlayerController::on_StopSendData_clicked()
{
	isInSendThread = false;
}

void PlayerController::run()
{
	qDebug() << "run thread begin";
	isInSendThread = true;
	FILE* fp = NULL;
	unsigned char* readBuff = NULL;
	fp = fopen(ui.FilePath->text().toLocal8Bit(), "rb");
	if (!fp)
	{
		qDebug() << ui.FilePath->text().toLocal8Bit().data() << " not exits";
		goto end;
	}
	readBuff = new unsigned char[10240];
	while (!isExit && isInSendThread && player)
	{
		int ret = fread(readBuff, 1, 10240, fp);
		if (ret <= 0)
		{
			qDebug() << "read ok";
			break;
		}

		while (!isExit && isInSendThread && player)
		{
			//动态库中已经添加了错误判断机制，包括player已经为空，在此过程中可以不用互斥锁
			if (PushStream2Cache(player, (char*)readBuff, ret))
			{
				break;
			}
			QThread::msleep(1);
			continue;
		}
		QThread::msleep(1);
	}

end:

	if (fp)
	{
		fclose(fp);
		fp = NULL;
	}
	
	if (readBuff)
	{
		delete readBuff;
		readBuff = NULL;
	}
	qDebug() << "run thread end";
	isInSendThread = false;
}

bool PlayerController::event(QEvent* event)
{
	if (event->type() == QtEvent::SetCanvas)
	{
		canvasMux.lock();
		if (canvas)
		{
			int scrWidth, scrHeight;
			GetScreenSize(&scrWidth, &scrHeight);
			//视频尺寸大于屏幕尺寸
			if (width > scrWidth || height > scrHeight)
			{
				float rate = (float)height / width;
				int w, h;
				//横屏
				if (width > height)
				{
					w = scrWidth * 0.7f;
					h = w * rate;
				}
				//竖屏
				else
				{
					h = scrHeight;
					w = h / rate;
				}
				canvas->resize(w, h);
			}
			else
			{
				canvas->resize(width, height);
			}

			canvas->Init(width, height);
		}
		canvasMux.unlock();
		isInSettingCanvas = false;
		return true;
	}
	return QWidget::event(event);
}

void PlayerController::GetScreenSize(int *width, int *height)
{

	QDesktopWidget *desktop = QApplication::desktop();
	int screenNum = desktop->screenCount();
	for (int i = 0; i < screenNum; i++)
	{
		QRect screen = desktop->screenGeometry();
		//qDebug("screen %d, width %d, height %d", i, screen.width(), screen.height());
		*width = screen.width();
		*height = screen.height();
	}

}