#include "ConvertTest.h"
#include <QDebug>

#include <QImage>
#include <QPainter>
#include <QPoint>
#include <QTimer>
#include <sys/timeb.h>
extern "C"
{
#include "../StreamDecoder/ConvertYUV.h"
}

#include "libyuv.h"
#pragma comment(lib, "yuv.lib")

ConvertTest::ConvertTest(QWidget *parent)
	: QWidget(parent)
{

	ui.setupUi(this);

	//初始化
	InitConverter();

	fp = fopen("F:/HTTPServer/4k.yuv", "rb");
	width = 3840;
	height = 2160;
	if (!fp)
	{
		qDebug() << "file open failed!";
		throw;
	}

	QTimer *timer = new QTimer(parent);
	timer->setSingleShot(false);
	timer->setInterval(20);
	timer->start();

	connect(timer, &QTimer::timeout, [this]() {
		on_DrawOneFrame_clicked();
	
	});
}

ConvertTest::~ConvertTest()
{
	if (fp) fclose(fp);
}

void ConvertTest::on_DrawOneFrame_clicked()
{

	needUpdate = true;
	update();
}

//获取时间戳ms 
long long GetTimestamp()
{
	struct timeb t;

	ftime(&t);

	return 1000 * t.time + t.millitm;
}

void ConvertTest::paintEvent(QPaintEvent *event)
{

	QPainter painter(this);
	painter.setBrush(Qt::black);
	QRect rect = ui.widget->geometry();
	painter.drawRect(rect);

	if (!needUpdate) return;
	needUpdate = false;
	if (feof(fp) != 0)return;

	
	int pxNum = width * height;
	if (!yBuf)
	{
		yBuf = new unsigned char[pxNum];
		uBuf = new unsigned char[pxNum / 4];
		vBuf = new unsigned char[pxNum / 4];
		rgbData = new unsigned char[pxNum * 4];
	}

	fread(yBuf, 1, pxNum, fp);
	fread(uBuf, 1, pxNum / 4, fp);
	fread(vBuf, 1, pxNum / 4, fp);

	long long t = GetTimestamp();
	libyuv::I420ToARGB(yBuf, width, uBuf, width / 2, vBuf, width / 2, rgbData, width * 4, width, height);
	//I420toRGB(yBuf, uBuf, vBuf, width, height, rgbData);
	qDebug() << "convert time:" << GetTimestamp() - t;
	//QT中的ARGB32模式在内存中的排列方式为BGRA
	QImage* mImg = new QImage(rgbData, width, height, QImage::Format::Format_ARGB32);

	
	///将图像按比例缩放成和窗口一样大小
	QImage img = mImg->scaled(ui.widget->size());

	painter.drawImage(QPoint(ui.widget->x(), ui.widget->y()), img);
	delete mImg;
	mImg = NULL;

}
