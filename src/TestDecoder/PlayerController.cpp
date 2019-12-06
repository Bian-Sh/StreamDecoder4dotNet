#include "PlayerController.h"
#include <QDebug>
#include <QFileDialog>
#include <QtConcurrent/QtConcurrent>
#include "StreamDecoder.h"
#include "Packet.h"
PlayerController::PlayerController(int playerID, QWidget *parent)
	: QWidget(parent), value(0x1122334455667788)
{
	ui.setupUi(this);
	ui.FilePath->setText("F:/HTTPServer/Faded.mp4");

	this->playerID = playerID;

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
	isExit = true;
	while (isInSendThread)
	{
		QThread::msleep(1);
	}
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
	if (pc)
	{
		pc->OnDrawFrameCb(frame);
	}
	
}
void sessionevent_callback(void* opaque, int playerID, int eventType)
{
	PlayerController* pc = (PlayerController*)opaque;
	if (pc)
	{
		pc->OnSessionEventCb(playerID, eventType);
	}
}

void PlayerController::OnDrawFrameCb(Frame * frame)
{
	//qDebug() << frame->pts;
}

void PlayerController::OnSessionEventCb(int playerID, int eventType)
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
	player = CreateSession(playerID);

	SetOption(player, OptionType::DataCacheSize, 1000000);

	SetEventCallBack(player, sessionevent_callback, drawframe_callback, this);
}
//删除一个Session
void PlayerController::on_DeleteSession_clicked()
{
	if (!player) return;
	DeleteSession(player);
	player = NULL;
}

void PlayerController::on_GetCacheFree_clicked()
{
	
	qDebug() << GetCacheFreeSize(player);
}

void PlayerController::on_TryBitStreamDemux_clicked()
{
	//player = new char(88);
	TryBitStreamDemux(player);
}

void PlayerController::on_TryNetStreamDemux_clicked()
{
	qDebug() << "on_TryNetStreamDemux_clicked";
}

void PlayerController::on_BeginDecode_clicked()
{
	BeginDecode(player);
}

void PlayerController::on_EndDecode_clicked()
{
	EndDecode(player);
}

void PlayerController::on_StartSendData_clicked()
{
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
	fp = fopen(ui.FilePath->text().toLatin1(), "rb");
	if (!fp)
	{
		qDebug() << ui.FilePath->text().toLatin1().data() << " not exits";
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
