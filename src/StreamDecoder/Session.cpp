#include "Session.h"
//#include <QDebug>
//#include <QMutexLocker>
#include "Decode.h"
#include "StreamDecoder.h"
#include "Packet.h"
#include <thread>
#include <iostream>
//#include <QCryptographicHash>
using namespace std;
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

void Sleep(int ms)
{
	chrono::milliseconds du(ms);
	this_thread::sleep_for(du);
}
char* Session::logbuf = NULL;
Session::Session(int dataCacheSize)
{
	av_register_all();
	avcodec_register_all();
	avformat_network_init();

	this->dataCacheSize = dataCacheSize;
	if (!dataCache) dataCache = new SCharList(dataCacheSize);
}

Session::~Session()
{
	if (logbuf)
	{
		delete logbuf;
		logbuf = NULL;
	}
	dataCacheMux.lock();
	delete dataCache;
	dataCache = NULL;
	dataCacheMux.unlock();
	Close();
	//wait();
	while (isInReadPacketThread)
		Sleep(1);

	cout << "~Session" << endl;
}


int ReadPacket(void *opaque, unsigned char *buf, int bufSize)
{
	Session* session = (Session*)opaque;
	size_t lastT = av_gettime();
	while (true)
	{
		if (session->isExit)
			return 0;

		session->dataCacheMux.lock();
		if (!session->dataCache)
		{
			session->dataCacheMux.unlock();
			return 0;
		}
		int size = session->dataCache->size();
		session->dataCacheMux.unlock();
		
		if (size > 0)
			break;

		Sleep(1);
		//超时退出
		if (session->isDemuxing)
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
				continue;
			}
		}
	}

	session->dataCacheMux.lock();
	int size = bufSize < session->dataCache->size() ? bufSize : session->dataCache->size();
	memcpy(buf, session->dataCache->arr, size);
	session->dataCache->pop_front(size);
	session->dataCacheMux.unlock();

	return size;
}

//添加数据流
bool Session::PushStream2Cache(char* data, int len)
{
	dataCacheMux.lock();
	if (!dataCache || dataCache->size() + len > dataCacheSize)
	{
		dataCacheMux.unlock();
		return false;
	}
	dataCache->push_back(data, len);
	dataCacheMux.unlock();
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


bool Session::OpenDemuxThread(int waitDemuxTime)
{
	//启动解封装线程失败， 线程正在运行
	if (isRuning)
	{
		StreamDecoder::Get()->PushLog2Net(LogLevel::Warning, "demux thread is run, please wait!");
		return false;
	}
	dataCache->Clear();
	this->waitDemuxTime = waitDemuxTime;
	isExit = false;
	isRuning = true;
	isDemuxing = true;
	std::thread th(&Session::Demux, this);
	th.detach();
	return true;
}

void Session::Demux()
{
	mux.lock();

	//这个readbuff定义为全局的会在第二次打开失败，原因未知
	unsigned char* readBuff = (unsigned char*)av_malloc(BUFF_SIZE);

	//创建 AVIOContext:
	if (!avio)
		avio = avio_alloc_context(readBuff, BUFF_SIZE, 0, this, ReadPacket, NULL, NULL);

	//探测流格式  
	//TODO要不要释放？
	AVInputFormat *piFmt = NULL;
	
	startTime = av_gettime();
	int ret = av_probe_input_buffer(avio, &piFmt, NULL, NULL, 0, 0);
	
	if (ret < 0)
	{
		mux.unlock();
		isRuning = false;
		isDemuxing = false;
		StreamDecoder::Get()->PushLog2Net(Warning, av_strerror2(ret));
		return;
	}
	char info[100];
	sprintf(info, "format:%s[%s]", piFmt->name, piFmt->long_name);
	StreamDecoder::Get()->PushLog2Net(Warning, info);

	afc = avformat_alloc_context();

	if (!afc)
	{
		mux.unlock();
		isRuning = false;
		isDemuxing = false;
		StreamDecoder::Get()->PushLog2Net(Warning, "avformat_alloc_context failed!");
		return;
	}
	afc->pb = avio;
	//The caller has supplied a custom AVIOContext, don't avio_close() it.
	//AVFMT_FLAG_CUSTOM_IO
	afc->flags = 0x0080;

	/*afc->interrupt_callback.opaque = this;
	afc->interrupt_callback.callback = interrupt_cb;*/

	ret = avformat_open_input(&afc, NULL, NULL, NULL);
	//av_dict_free(&opt);
	if (ret < 0)
	{
		mux.unlock();
		isRuning = false;
		isDemuxing = false;
		StreamDecoder::Get()->PushLog2Net(Warning, "avformat_open_input failed!");
		return;
	}
	cout << "open Success" << endl;
	//获取流信息
	ret = avformat_find_stream_info(afc, NULL);
	if (ret < 0)
	{
		mux.unlock();
		isRuning = false;
		isDemuxing = false;
		StreamDecoder::Get()->PushLog2Net(Warning, av_strerror2(ret));
		return;
	}
	//打印流详细信息
	//av_dump_format(afc, 0, NULL, 0);

	if (afc->nb_streams == 2)
	{
		audioStreamIndex = av_find_best_stream(afc, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
		StreamDecoder::Get()->PushLog2Net(Warning, "this stream is containt AudioStream");
	}

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
		mux.unlock();
		isRuning = false;
		isDemuxing = false;
		StreamDecoder::Get()->PushLog2Net(Warning, "open decode failed!");
		return;
	}
	/*DecodeEvent* trigger = new DecodeEvent(DecodeEvent::DemuxSuccess);
	QCoreApplication::postEvent(this, trigger);*/
	mux.unlock();
	isDemuxing = false;
	StreamDecoder::Get()->PushLog2Net(Warning, "Demux Success!");

	
}

void Session::BeginDecode()
{
	if (isDemuxing)
	{
		cout << "demuxing please wait!" << endl;
		return;
	}
	if (!isRuning)
	{
		cout << "demux not ok!" << endl;
		return;
	}
	if (isInReadPacketThread)
	{
		cout << "decodeing please wait" << endl;
		return;
	}

	std::thread dth(&Decode::run, decode);
	dth.detach();

	std::thread th(&Session::run, this);
	th.detach();
	cout << "begin decode success" << endl;
}

void Session::StopDecode()
{
	if (!isInReadPacketThread) return;
	cout << "stop decode" << endl;

	Close();
}

void Session::Close()
{
	isExit = true;

	mux.lock();

	if (decode)
	{
		decode->Close();
		decode = NULL;
	}

	/*dataCacheMux.lock();
	dataCache->Clear();
	dataCacheMux.unlock();*/

	/*if (yuv[0])
	{
		delete yuv[0];
		delete yuv[1];
		delete yuv[2];
		yuv[0] = NULL;
		yuv[1] = NULL;
		yuv[2] = NULL;
		linesizeY = 0;
	}*/

	width = 0;
	height = 0;

	if (avio)
	{
		//释放AVIOContext 并置0
		avio_context_free(&avio);
	}
	if (afc)
	{
		//avformat_flush(afc);
		avformat_close_input(&afc);
	}
		
	mux.unlock();
	videoStreamIndex = audioStreamIndex = -1;
	isRuning = false;
}

int Session::GetCacheFreeSize()
{
	return dataCacheSize - dataCache->size();
}

char* Session::av_strerror2(int errnum)
{
	if (logbuf == NULL) logbuf = new char[1024];
	memset(logbuf, 0, 1024);
	av_strerror(errnum, logbuf, 1023);
	return logbuf;
}

void Session::OnDecodeOnFrame(AVFrame *frame)
{
	mux.lock();
	if (isExit)
	{
		mux.unlock();
		av_frame_free(&frame);
		return;
	}

	/*if (linesizeY != frame->linesize[0])
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
	memcpy(yuv[2], frame->data[2], width * height / 4);*/

	Frame *_fr = new Frame(frame->width, frame->height, (char*)frame->data[0], (char*)frame->data[1], (char*)frame->data[2]);
	StreamDecoder::Get()->PushFrame2Net(_fr);

	mux.unlock();
	av_frame_free(&frame);

}

void Session::run()
{
	isInReadPacketThread = true;
	while (!isExit)
	{

		mux.lock();
		if (!afc)
		{
			mux.unlock();
			break;
		}
			

		AVPacket* pkt = av_packet_alloc();
		//qDebug() << "next read";
		int ret = av_read_frame(afc, pkt);
		//qDebug() << "read a frame";
		if (ret != 0)
		{
			mux.unlock();
			//读取到结尾
			cout << "read end!!" << endl;
			av_packet_free(&pkt);
			cout << av_strerror2(ret) << endl;
			break;
		}
		//丢弃音频
		if (audioStreamIndex >= 0 && pkt->stream_index == audioStreamIndex)
		{
			mux.unlock();
			av_packet_free(&pkt);
			Sleep(1);
			continue;
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
		mux.unlock();
		//av_packet_free(&pkt);
		Sleep(1);

	}
	cout << "read packet end" << endl;
	isInReadPacketThread = false;
}