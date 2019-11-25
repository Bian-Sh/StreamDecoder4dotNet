#pragma once

#include <QtWidgets/QWidget>
#include "ui_H264Decoder.h"
#include "Session.h"
#include <QThread>
#include <QMutex>


void OnDraw(struct DotNetFrame* frame);
void OnEvent(int playerID, int eventType);
class H264Decoder : public QWidget
{
	Q_OBJECT

public:
	H264Decoder(QWidget *parent = Q_NULLPTR);

	static H264Decoder* self;
	void OnDrawFrame(DotNetFrame* frame);
	void OnEventPkt(int playerID, int eventType);
public slots :
	void on_CreateSession_clicked();
	void on_DeleteSession_clicked();
	void on_TryBitStreamDemux_clicked();
	void on_TryNetStreamDemux_clicked();
	void on_BeginDecode_clicked();
	void on_StopDecode_clicked();

	void on_GetFree_clicked();

	void on_OpenFile_clicked();

	void on_StartSendData_clicked();

	void on_EndSendData_clicked();


	void OnRead();
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

	//QString filePath = "d:/HTTPServer/faded.MP4";
	//QString filePath = "F:/HTTPServer/faded10s.h264";
	//QString filePath = "D:/device.h264";
	QString filePath = "F:/HTTPServer/4K.mp4";

private:
	class QTcpServer* mServer = NULL;
	class QTcpSocket* mSocket = NULL;

	bool GetDeviceInfoOnConnect(QString &deviceName, QSize &size);

};
