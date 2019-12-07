#include "TestDecoder.h"
#include <QDebug>
#include <QTimer>
#include "StreamDecoder.h"
#include <QCryptographicHash>
#include <QCoreApplication>
#include "PlayerController.h"

#pragma comment(lib, "StreamDecoder.lib")


TestDecoder::TestDecoder(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);


	QTimer *timer = new QTimer(parent);
	timer->setSingleShot(false);
	timer->setInterval(10);
	//timer->start();

	connect(timer, &QTimer::timeout, [this]() {

	});

	StreamDecoderInitialize([](int level, char* log) {
		qDebug() << log;
	});

}

TestDecoder::~TestDecoder()
{
	closeEvent(NULL);
}
void TestDecoder::closeEvent(QCloseEvent *event)
{
	int size = playerList.size();
	for (int i = 0; i < size; i++)
	{
		PlayerController* p = playerList.front();
		playerList.pop_front();
		
		if (p->isVaild())
		{
			p->deleteLater();
			p = NULL;
		}
		
	}

}

void TestDecoder::OnRead()
{
	
}



void TestDecoder::on_CreatePlayer_clicked()
{
	/*int id = 0;
	while (std::count(playerIDvector.begin(), playerIDvector.end(), id) > 0)
	{
		id++;
	}
	playerIDvector.push_back(id);*/

	PlayerController* p = new PlayerController();
	p->show();
	playerList.push_back(p);
}


bool TestDecoder::GetDeviceInfoOnConnect(QString &deviceName, QSize &size)
{
	//// abk001----------0xaaaa 0xbbbb
	//// 64字节设备名称   2字节宽 2字节高
	//unsigned char buf[68];
	//if (mSocket->bytesAvailable() <= (68))
	//{
	//	//等待300毫秒
	//	mSocket->waitForReadyRead(300);
	//}
	//qint64 len = mSocket->read((char*)buf, sizeof(buf));
	//if (len < 68)
	//{
	//	qInfo("Could not retrieve device infomation");
	//	return false;
	//}
	//buf[63] = '\0';
	//deviceName = (char*)buf;
	////0x0438
	////第64个是0x04  左移8位为0x0400
	//size.setWidth((buf[64] << 8) | buf[64 + 1]);
	//size.setHeight(buf[64 + 2] << 8 | buf[64 + 3]);
	return true;
}

