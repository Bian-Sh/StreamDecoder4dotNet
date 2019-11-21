#include "Session.h"
#include "Decode.h"
#include "StreamDecoder.h"
#include "Packet.h"
#include <iostream>
#include "Tools.h"
using namespace std;
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libavutil/time.h>
}
#define BUFF_SIZE 655360
#define  URL_LENGTH 128
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")

int ReadPacket(void *opaque, unsigned char *buf, int bufSize)
{
	Session* session = (Session*)opaque;
	if (!session) return 0;
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

		Tools::Get()->Sleep(1);
		//超时退出
		if (session->isDemuxing)
		{
			
			if (av_gettime() - session->startTime > session->waitDemuxTime * 1000)
			{
				//cout << "return 0" << endl;
				return 0;
			}
			else
			{
				//cout << "continue" << endl;
				continue;
			}
		}
		else
		{
			//1s读不到数据认为流中断
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

Session::Session(int dataCacheSize)
{
	//初始化 数据流 缓冲
	this->dataCacheSize = dataCacheSize;
	if (!dataCache) dataCache = new SCharList(this->dataCacheSize);

	//初始化 视频流地址 数组
	url = new char[URL_LENGTH];
	memset(url, 0, URL_LENGTH);

	//unsigned char* readBuff = (unsigned char*)av_malloc(BUFF_SIZE);
	//avio = avio_alloc_context(readBuff, BUFF_SIZE, 0, this, ReadPacket, NULL, NULL);

}

Session::~Session()
{
	Close();
	cout << "~Session" << endl;
}
void Session::Close()
{
	Clear();
	dataCacheMux.lock();
	delete dataCache;
	dataCache = NULL;
	dataCacheMux.unlock();

	mux.lock();
	delete url;
	url = NULL;


	
	mux.unlock();

}

void Session::Clear()
{

	isExit = true;

	mux.lock();

	if (afc)
	{
		//释放并置零 内部会调用avformat_free_context
		avformat_close_input(&afc);
		//avformat_free_context(afc);
	}
	
	if (avio)
	{
		av_free(avio->buffer);

		//释放并置零
		avio_context_free(&avio);
	}
	
	if (decode)
	{
		delete decode;
		decode = NULL;
	}

	videoStreamIndex = audioStreamIndex = -1;
	mux.unlock();

	//等待线程退出
	while (isInReadPacketThread)
		Tools::Get()->Sleep(1);

	while (isProbeBuffer)
		Tools::Get()->Sleep(1);

	isRuning = false;
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
int interrupt_cb(void *ctx)
{
	Session *session = (Session*)ctx;
	if (session->isExit) return 1;
	//cout << "interrupt_cb" << endl;
	return 0;
}

//尝试打开bit流解封装线程
bool Session::TryBitStreamDemux(int waitDemuxTime)
{
	if (!OpenDemuxThread()) return false;

	//把url清空
	memset(url, 0, URL_LENGTH);

	std::thread t(&Session::ProbeInputBuffer, this);
	t.detach();

	return true;
}
//尝试打开网络流解封装线程
bool Session::TryNetStreamDemux(char* url)
{
	if (!OpenDemuxThread()) return false;
	memset(this->url, 0, URL_LENGTH);
	memcpy(this->url, url, strlen(url));
	StreamDecoder::Get()->PushLog2Net(Info, this->url);
	afc = avformat_alloc_context();
	afc->interrupt_callback.opaque = this;
	afc->interrupt_callback.callback = interrupt_cb;

	std::thread th(&Session::Demux, this);
	th.detach();
	return true;
}

bool Session::OpenDemuxThread()
{
	//启动解封装线程失败， 线程正在运行
	if (isRuning)
	{
		StreamDecoder::Get()->PushLog2Net(LogLevel::Warning, "Current session is runing, please wait!");
		return false;
	}
	dataCache->Clear();
	this->waitDemuxTime = waitDemuxTime;
	isExit = false;
	isRuning = true;
	
	//创建AVFormatContext
	if (afc) {
		cout << "严重错误 afc 存在" << endl;
		return false;
	}
	afc = avformat_alloc_context();
	if (!afc)
	{
		StreamDecoder::Get()->PushLog2Net(Warning, "avformat_alloc_context failed!");
		return false;
	}
	return true;
}

void Session::ProbeInputBuffer()
{
	isDemuxing = true;

	//探测流格式  
	//TODO要不要释放？
	AVInputFormat *piFmt = NULL;
	//AVInputFormat* in_fmt = av_find_input_format("h265");

	unsigned char* readBuff = (unsigned char*)av_malloc(BUFF_SIZE);

	//创建 AVIOContext， 使用avio_context_free()释放并置零, readBuff释放用av_free(avio->buffer) 不要定义全局的变量存储，释放全局的会出错
	if (avio)
	{
		Clear();
		cout << "严重错误 avio 存在" << endl;
		isRuning = false;
		isDemuxing = false;
		return;
	}
	avio = avio_alloc_context(readBuff, BUFF_SIZE, 0, this, ReadPacket, NULL, NULL);
	if (!avio)
	{
		Clear();
		StreamDecoder::Get()->PushLog2Net(Warning, "avio_alloc_context failed!");
		isRuning = false;
		isDemuxing = false;
		return;
	}
	afc->pb = avio;
	
	//The caller has supplied a custom AVIOContext, don't avio_close() it.
	//AVFMT_FLAG_CUSTOM_IO
	afc->flags = AVFMT_FLAG_CUSTOM_IO;

	startTime = av_gettime();
	isProbeBuffer = true;
	int ret = av_probe_input_buffer(avio, &piFmt, NULL, NULL, 0, 0);
	isProbeBuffer = false;
	isUseReadBuff = true;
	if (ret < 0)
	{
		Clear();
		StreamDecoder::Get()->PushLog2Net(Warning, Tools::Get()->av_strerror2(ret));
		isRuning = false;
		isDemuxing = false;
		return;
	}

	char info[100];
	sprintf(info, "format:%s[%s]", piFmt->name, piFmt->long_name);
	StreamDecoder::Get()->PushLog2Net(Warning, info);

	Demux();
}



void Session::Demux()
{

	//AVDictionary *opts = NULL;
	//设置rtsp流已tcp协议打开
	//av_dict_set(&opts, "rtsp_transport", "tcp", 0);
	//网络延时时间
	//av_dict_set(&opts, "max_delay", "500", 0);
	//av_dict_set_int(&opts, "stimeout", 5000, 0);

	int ret = avformat_open_input(&afc, url, NULL, NULL);
	//av_dict_free(&opts);
	if (ret < 0)
	{
		Clear();
		StreamDecoder::Get()->PushLog2Net(Warning, "avformat_open_input failed!");
		isRuning = false;
		isDemuxing = false;
		return;
	}
	cout << "avformat_open_input open Success" << endl;
	//读取一段视频 获取流信息
	ret = avformat_find_stream_info(afc, NULL);
	if (ret < 0)
	{
		Clear();
		StreamDecoder::Get()->PushLog2Net(Warning, Tools::Get()->av_strerror2(ret));
		isRuning = false;
		isDemuxing = false;
		return;
	}

	//打印流详细信息
	av_dump_format(afc, 0, NULL, 0);

	if (afc->nb_streams >= 2)
	{
		audioStreamIndex = av_find_best_stream(afc, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
		if(audioStreamIndex != -1)
			StreamDecoder::Get()->PushLog2Net(Warning, "this stream is containt AudioStream");
	}

	videoStreamIndex = av_find_best_stream(afc, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);


	/*AVCodecParameters *para = avcodec_parameters_alloc();
	avcodec_parameters_copy(para, afc->streams[videoStreamIndex]->codecpar);*/
	width = afc->streams[videoStreamIndex]->codecpar->width;
	height = afc->streams[videoStreamIndex]->codecpar->height;
	/*if (!decode->Open(afc->streams[videoStreamIndex]->codecpar))
	{
		Clear();
		StreamDecoder::Get()->PushLog2Net(Warning, "open decode failed!");
		isRuning = false;
		isDemuxing = false;
		return;
	}*/

	isDemuxing = false;
	StreamDecoder::Get()->PushLog2Net(Warning, "Demux Success!");

}

void Session::BeginDecode()
{
	
	//正在解封装
	if (isDemuxing)
	{
		cout << "Please wait demuxing!" << endl;
		return;
	}
	//解封装失败
	if (!isRuning)
	{
		cout << "Demux failed!" << endl;
		return;
	}
	if (!decode)
	{
		cout << "解码器不存在" << endl;
		return;
	}

	std::thread decode_t(&Decode::run, decode);
	decode_t.detach();

	std::thread t(&Session::run, this);
	t.detach();

	cout << "Begin decode success" << endl;
}

void Session::StopDecode()
{
	if (!isRuning)
	{
		cout << "Don't need stop decode" << endl;
		return;
	}
	cout << "Stop decode" << endl;
	Clear();
}



int Session::GetCacheFreeSize()
{
	return dataCacheSize - dataCache->size();
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

	//不需要内存对齐
	if (frame->linesize[0] == width)
	{
		Frame *tmpFrame = new Frame(frame->width, frame->height, (char*)frame->data[0], (char*)frame->data[1], (char*)frame->data[2]);
		StreamDecoder::Get()->PushFrame2Net(tmpFrame);
	}
	else
	{
		Frame *tmpFrame = new Frame(width, height, NULL, NULL, NULL, false);
		for (int i = 0; i < height; i++)
		{
			memcpy(tmpFrame->frame_y + width * i, frame->data[0] + frame->linesize[0] * i, width);
		}
		for (int i = 0; i < height / 2; i++)
		{
			memcpy(tmpFrame->frame_u + width / 2 * i, frame->data[1] + frame->linesize[1] * i, width / 2);
		}
		for (int i = 0; i < height / 2; i++)
		{
			memcpy(tmpFrame->frame_v + width / 2 * i, frame->data[2] + frame->linesize[2] * i, width / 2);
		}
		StreamDecoder::Get()->PushFrame2Net(tmpFrame);

	}

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
			cout << Tools::Get()->av_strerror2(ret) << endl;
			break;
		}
		//丢弃音频
		if (audioStreamIndex >= 0 && pkt->stream_index == audioStreamIndex)
		{
			mux.unlock();
			av_packet_free(&pkt);
			Tools::Get()->Sleep(1);
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
		Tools::Get()->Sleep(1);

	}
	cout << "Session Thread Quit" << endl;
	isInReadPacketThread = false;
}