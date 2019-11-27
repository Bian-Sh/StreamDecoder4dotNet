#include "I420toRGB.h"
#include <QDebug>
#include "ConvertToRGB.h"
#include <QImage>
#include <QPainter>
#include <QPoint>
I420toRGB::I420toRGB(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	convert = new ConvertToRGB();

	fp = fopen("F:/HTTPServer/ExcuseMe_852x480.yuv", "rb");
	if (!fp)
	{
		qDebug() << "file open failed!";
		throw;
	}
}

I420toRGB::~I420toRGB()
{

}

void I420toRGB::on_DrawOneFrame_clicked()
{
	QImage* img = new QImage(100, 100, QImage::Format_ARGB32);
	
	update();
}

void I420toRGB::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.setBrush(Qt::black);
	QRect rect = ui.widget->geometry();
	painter.drawRect(rect);

	///将图像按比例缩放成和窗口一样大小
	QImage img = mImg.scaled(ui.widget->size());
	//QImage img = mImage;
	int x = ui.widget->width() - img.width();
	int y = ui.widget->height() - img.height();
	x /= 2;
	y /= 2;
	x += ui.widget->x();
	y += ui.widget->y();
	painter.drawImage(QPoint(x, y), img);
}
