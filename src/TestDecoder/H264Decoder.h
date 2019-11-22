#pragma once

#include <QtWidgets/QWidget>
#include "ui_H264Decoder.h"
#include "Session.h"
#include <QThread>
#include <QMutex>

void OnDraw(struct DotNetFrame* frame);
class H264Decoder : public QWidget
{
	Q_OBJECT

public:
	H264Decoder(QWidget *parent = Q_NULLPTR);

	static H264Decoder* self;
	void OnDrawFrame(DotNetFrame* frame);
public slots :
	void on_createsession_clicked();
	void on_deletesession_clicked();
	void on_trydemux_clicked();
	void on_trynetstreamdemux_clicked();
	void on_begindecode_clicked();
	void on_stopdecode_clicked();

	void on_GetFree_clicked();

	void on_openFile_clicked();

	void on_StartSendData_clicked();

	void on_EndSendData_clicked();


protected:
	void closeEvent(QCloseEvent *event);
private:
	Ui::H264DecoderClass ui;

	void* session = NULL;

	char* readBuff = NULL;
	FILE *fp = NULL;
	void mrun();

	bool isExit = true;

	class DrawI420* canvas = NULL;

	int width;
	int height;

	QString filePath = "F:/HTTPServer/faded10s.h264";
};
