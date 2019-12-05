#include "TestDecoder.h"
#include <QtWidgets/QApplication>
#include <QDebug>
#include <sys/timeb.h>
#include "PlayerController.h"
//#include <sys/timeb.h>
//获取时间戳ms 
long long GetNowMs()
{
	struct timeb t;

	ftime(&t);

	return 1000 * t.time + t.millitm;
}



int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	
	TestDecoder w;
	w.show();


	return a.exec();
}
