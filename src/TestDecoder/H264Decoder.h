#pragma once

#include <QtWidgets/QWidget>
#include "ui_H264Decoder.h"
#include "Session.h"
#include <QThread>
#include <QMutex>
class H264Decoder : public QWidget
{
	Q_OBJECT

public:
	H264Decoder(QWidget *parent = Q_NULLPTR);

public slots :
	void on_createsession_clicked();
	void on_deletesession_clicked();
	void on_OpenDemux_clicked();
	void on_begindecode_clicked();
	void on_stopdecode_clicked();

	void on_GetFree_clicked();
protected:
	void closeEvent(QCloseEvent *event);
private:
	Ui::H264DecoderClass ui;

	void* session = NULL;

	char* readBuff = NULL;
	FILE *fp = NULL;
	void mrun();

	bool isExit = false;

	bool isRunthread = false;
};
