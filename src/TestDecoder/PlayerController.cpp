#include "PlayerController.h"
#include <QDebug>
#include <QFileDialog>
#include <QtConcurrent/QtConcurrent>
#include "StreamDecoder.h"
PlayerController::PlayerController(int playerID, QWidget *parent)
	: QWidget(parent), value(0x1122334455667788)
{
	ui.setupUi(this);
	ui.FilePath->setText(filePath);

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
	filePath = QFileDialog::getOpenFileName(this, "select file", "F:/HTTPServer");
	ui.FilePath->setText(filePath);
}

//创建一个Session
void PlayerController::on_CreateSession_clicked()
{
	if (player) return;
	player = CreateSession(playerID);
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
	qDebug() << "on_BeginDecode_clicked";
}

void PlayerController::on_EndDecode_clicked()
{
	qDebug() << "on_EndDecode_clicked";
}

void PlayerController::on_StartSendData_clicked()
{
	if (isInSendThread || isExit) return;
	QtConcurrent::run(this, &PlayerController::run);
}

void PlayerController::on_StopSendData_clicked()
{
	qDebug() << "on_StopSendData_clicked";
}

void PlayerController::run()
{
	isInSendThread = true;
	FILE* fp = NULL;
	unsigned char* readBuff = NULL;
	fp = fopen(filePath.toLatin1(), "rb");
	if (!fp)
	{
		qDebug() << filePath.toLatin1().data() << " not exits";
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

			if (PushStream2Cache(player, (char*)readBuff, ret))
			{
				break;
			}
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
