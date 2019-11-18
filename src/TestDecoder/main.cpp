#include "H264Decoder.h"
#include <QtWidgets/QApplication>
#include <QDebug>
#include <sys/timeb.h>

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
	
	H264Decoder w;
	w.show();


	
	

	return a.exec();
}
