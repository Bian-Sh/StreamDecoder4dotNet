#pragma once

#include <QtWidgets/QWidget>
#include "ui_I420toRGB.h"
#include <QImage>
class ConvertToRGB;
class I420toRGB : public QWidget
{
	Q_OBJECT

public:
	I420toRGB(QWidget *parent = Q_NULLPTR);
	~I420toRGB();

	ConvertToRGB* convert;

public slots:

	void on_DrawOneFrame_clicked();

protected:
	void paintEvent(QPaintEvent *event);

private:
	Ui::I420toRGBClass ui;

	FILE* fp = NULL;

	QImage mImg;
};
