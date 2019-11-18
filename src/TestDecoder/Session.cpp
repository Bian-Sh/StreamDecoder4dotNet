#include "Session.h"
#include <QDebug>
#include <QMutexLocker>
#include "Decode.h"
#include <QWidget>
#include "DrawI420.h"
#include <QtConcurrent/QtConcurrent>
#include "DecodeEvent.h"
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libavutil/time.h>
}
#define BUFF_SIZE 65536
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")

#define USE_WIDGET

Session::Session()
{
	av_register_all();
	avcodec_register_all();
	avformat_network_init();

	connect(this, &Session::OnDemuxSuccessSignal, [this]() {
		qDebug() << "DemuxSuccess";
#ifdef USE_WIDGET
		canvas = new DrawI420(this);
		canvas->resize(width, height);
		canvas->show();
		canvas->Init(width, height);
#endif
	});
}

Session::~Session()
{
	qDebug() << "~Session";
	Close();
	wait();
}


int ReadPacket(void *opaque, unsigned char *buf, int bufSize)
{
	Session* session = (Session*)opaque;

	size_t lastT = av_gettime();
	while (true)
	{
		if (session->isExit)
			return 0;

		if (session->dataCache.size() > 0)
			break;

		QThread::msleep(1);
		//超时退出
		if (session->demuxing)
		{
			if (av_gettime() - session->startTime > session->waitDemuxTime * 1000)
			{
				return 0;
			}
			else
			{
				continue;
			}
		}
		else
		{
			if (av_gettime() - lastT > 1000 * 1000)
			{
				return 0;
			}
			else
			{
				//qDebug() << av_gettime() - lastT;
				continue;
			}
		}
		
			
	}

	session->dataCacheMux.lock();
	int size = qMin((size_t)bufSize, (size_t)session->dataCache.size());
	memcpy(buf, session->dataCache, size);
	session->dataCache.remove(0, size);
	session->dataCacheMux.unlock();
	return size;
}
//添加数据流
bool Session::PushStream2Cache(char* data, int len)
{
	QMutexLocker locker(&dataCacheMux);
	if (dataCache.size() > dataCacheSize) return false;
	dataCache.append(data, len);
	return true;
}
//static int interrupt_cb(void *ctx)
//{
//	Session *session = (Session*)ctx;
//	qDebug() << "interrupt_cb";
//	int  timeout = 3;
//	if (av_gettime() - session->lastreadpackettime > timeout * 1000 * 1000)
//	{
//		return 1;
//	}
//	return 0;
//}
bool Session::OpenDemuxThread()
{
	//启动解封装线程失败， 线程正在运行
	if (isRun)
	{
		qDebug() << "demux thread is run, please wait!";
		return false;
	}
	isExit = false;
	isRun = true;
	QtConcurrent::run(this, &Session::OpenThread);
	return true;
}

void Session::OpenThread()
{
	QMutexLocker locker(&mux);


	//这个readbuff定义为全局的会在第二次打开失败，原因未知
	unsigned char* readBuff = (unsigned char*)av_malloc(BUFF_SIZE);

	//创建 AVIOContext:
	if (!avio)
		avio = avio_alloc_context(readBuff, BUFF_SIZE, 0, this, ReadPacket, NULL, NULL);

	//探测流格式  
	//TODO要不要释放？
	AVInputFormat *piFmt = NULL;
	demuxing = true;
	startTime = av_gettime();
	int ret = av_probe_input_buffer(avio, &piFmt, NULL, NULL, 0, 0);
	demuxing = false;
	if (ret < 0)
	{
		av_strerror2(ret);
		qDebug() << "av_probe_input_buffer2 failed";
		return;
	}
	qDebug() << "probe success!";
	qDebug() << QString("format:%0[%1]").arg(piFmt->name, piFmt->long_name).toUtf8().data();


	afc = avformat_alloc_context();

	if (!afc)
	{
		qDebug() << "avformat_alloc_context failed";
		return;
	}
	afc->pb = avio;
	//The caller has supplied a custom AVIOContext, don't avio_close() it.
	afc->flags = AVFMT_FLAG_CUSTOM_IO;

	/*afc->interrupt_callback.opaque = this;
	afc->interrupt_callback.callback = interrupt_cb;*/

	ret = avformat_open_input(&afc, NULL, NULL, NULL);
	//av_dict_free(&opt);
	if (ret < 0)
	{
		qDebug() << "avformat_open_input failed!";
		return;
	}
	qDebug() << "open Success";
	//获取流信息
	ret = avformat_find_stream_info(afc, NULL);
	if (ret < 0)
	{
		av_strerror2(ret);
		return;
	}
	//打印流详细信息
	//av_dump_format(afc, 0, NULL, 0);


	videoStreamIndex = av_find_best_stream(afc, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);

	if (decode == NULL)
	{
		decode = new Decode(this);
	}
	AVCodecParameters *para = avcodec_parameters_alloc();
	avcodec_parameters_copy(para, afc->streams[videoStreamIndex]->codecpar);
	width = para->width;
	height = para->height;
	if (!decode->Open(para))
	{
		qDebug() << "open decode failed!";
		return;
	}
	DecodeEvent* trigger = new DecodeEvent(DecodeEvent::DemuxSuccess);
	QCoreApplication::postEvent(this, trigger);

	decode->start();
	start();
}

void Session::Close()
{
	isExit = true;
	qDebug() << "Session::Close";

	QMutexLocker locker(&mux);

	if (decode)
	{
		decode->Close();
		decode = NULL;
	}

#ifdef USE_WIDGET
	if (canvas)
	{
		canvas->Close();
		canvas->deleteLater();
		canvas = NULL;
	}
#endif

	dataCacheMux.lock();
	dataCache.clear();
	dataCacheMux.unlock();

	if (yuv[0])
	{
		delete yuv[0];
		delete yuv[1];
		delete yuv[2];
		yuv[0] = NULL;
		yuv[1] = NULL;
		yuv[2] = NULL;
		linesizeY = 0;
	}

	width = 0;
	height = 0;

	if (avio)
	{
		//avio_close(avio);
		qDebug() << "close avio";
		//释放AVIOContext 并置0
		avio_context_free(&avio);
	}
	if(afc)
		avformat_close_input(&afc);

	isRun = false;
}

int Session::GetCacheFreeSize()
{
	QMutexLocker locker(&mux);
	return dataCacheSize - dataCache.size();
}

void Session::av_strerror2(int errnum)
{
	char buf[100] = { 0 };
	av_strerror(errnum, buf, sizeof(buf) - 1);
	qDebug() << buf;
}

void Session::OnDecodeOnFrame(AVFrame *frame, bool use_frame)
{
	QMutexLocker locker(&mux);
	if (isExit)
	{
		av_frame_free(&frame);
		return;
	}
	
	if (linesizeY != frame->linesize[0])
	{
		width = frame->width;
		height = frame->height;
		if (yuv[0])
		{
			delete yuv[0];
			delete yuv[1];
			delete yuv[2];
		}
		linesizeY = frame->linesize[0];

		yuv[0] = new unsigned char[width * height];
		yuv[1] = new unsigned char[width * height / 4];
		yuv[2] = new unsigned char[width * height / 4];
	}
	memcpy(yuv[0], frame->data[0], width * height);
	memcpy(yuv[1], frame->data[1], width * height / 4);
	memcpy(yuv[2], frame->data[2], width * height / 4);

	av_frame_free(&frame);

#ifdef USE_WIDGET
	while (canvas)
	{
		if (isExit) break;;
		if (!canvas->isValid || canvas->Repaint(yuv))
			break;
		QThread::msleep(1);
	}
#endif
}

void Session::run()
{
	while (!isExit)
	{

		QMutexLocker locker(&mux);
		if (!afc)
			break;

		AVPacket* pkt = av_packet_alloc();
		int ret = av_read_frame(afc, pkt);
		if (ret != 0)
		{
			//读取到结尾
			qDebug() << "read end!!";
			av_packet_free(&pkt);
			av_strerror2(ret);
			break;
		}
		//读取到一个AVPacket
		if (decode)
		{
			decode->Push(pkt);
		}
		else
		{
			av_packet_free(&pkt);
		}
		//av_packet_free(&pkt);
		QThread::msleep(1);

	}
	qDebug() << "read packet end";
}

bool Session::event(QEvent* ev)
{
	if (ev->type() == DecodeEvent::DemuxSuccess)
	{
		emit OnDemuxSuccessSignal();
		return true;
	}
	if (ev->type() == DecodeEvent::CloseWidget)
	{
		Close();
		return true;
	}
	return QThread::event(ev);
}