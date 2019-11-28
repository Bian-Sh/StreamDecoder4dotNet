#pragma once

#include <QtWidgets/QWidget>
#include "ui_I420toRGB.h"
#include <QImage>
//class ConvertToRGB;
class ConvertTest : public QWidget
{
	Q_OBJECT

public:
	ConvertTest(QWidget *parent = Q_NULLPTR);
	~ConvertTest();

	//ConvertToRGB* convert;

public slots:

	void on_DrawOneFrame_clicked();

protected:
	void paintEvent(QPaintEvent *event);

private:
	Ui::I420toRGBClass ui;

	FILE* fp = NULL;

	bool needUpdate = false;

	unsigned char* yBuf = NULL;
	unsigned char* uBuf = NULL;
	unsigned char* vBuf = NULL;

	unsigned char* rgbData = NULL;

	int width = 0;
	int height = 0;
};
