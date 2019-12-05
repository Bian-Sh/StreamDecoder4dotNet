#pragma once

#include <QtWidgets/QWidget>
#include "ui_TestDecoder.h"
#include "Session.h"
#include <QThread>
#include <QMutex>

class PlayerController;

class TestDecoder : public QWidget
{
	Q_OBJECT

public:
	TestDecoder(QWidget *parent = Q_NULLPTR);
	~TestDecoder();
public slots :

	void on_CreatePlayer_clicked();

	void OnRead();

protected:

	void closeEvent(QCloseEvent *event);

	bool event(QEvent* event);

private:

	Ui::H264DecoderClass ui;


	class QTcpServer* mServer = NULL;
	class QTcpSocket* mSocket = NULL;

	bool GetDeviceInfoOnConnect(QString &deviceName, QSize &size);

private:

	std::vector<int> playerIDvector;
	std::list<PlayerController*> playerList;
};
