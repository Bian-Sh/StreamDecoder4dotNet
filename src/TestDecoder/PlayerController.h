#pragma once

#include <QWidget>
#include <mutex>
#include "ui_PlayerController.h"

struct Frame;
class PlayerController : public QWidget
{
	Q_OBJECT

public:
	PlayerController(int playerID, QWidget *parent = Q_NULLPTR);
	~PlayerController();

	//一个简单验证指针是否有效的方法
	bool isVaild();

	void OnDrawFrameCb(Frame * frame);
	void OnSessionEventCb(int playerID, int eventType);
protected:

	void closeEvent(QCloseEvent *event);

private slots:

	void on_OpenFile_clicked();
	
	void on_CreateSession_clicked();

	void on_DeleteSession_clicked();

	void on_GetCacheFree_clicked();

	void on_TryBitStreamDemux_clicked();

	void on_TryNetStreamDemux_clicked();

	void on_BeginDecode_clicked();

	void on_EndDecode_clicked();

	void on_StartSendData_clicked();

	void on_StopSendData_clicked();

private:

	void run();

private:
	Ui::PlayerController ui;

	long long value;

	bool isExit = false;
	bool isInSendThread = false;

	int playerID;
	void* player = NULL;
};
