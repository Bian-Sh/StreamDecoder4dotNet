#include "I420toRGB.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	I420toRGB w;
	w.show();
	return a.exec();
}
