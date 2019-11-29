#include "Demux.h"
#include <iostream>
#include "StreamDecoder.h"
#include "Tools.h"
#include "Packet.h"
#include "Session.h"
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libavutil/time.h>
}
#define BUFF_SIZE 65536
#define  URL_LENGTH 1280
using namespace  std;
Demux::Demux(Session* session)
{
	this->session = session;
	this->dataCacheSize = session->config->dataCacheSize;
	//初始化 视频流地址 数组
	url = new char[URL_LENGTH];
	memset(url, 0, URL_LENGTH);

	dataCache = new SCharList(this->dataCacheSize);
}


Demux::~Demux()
{
	Close();
	cout << "~Demux" << endl;
}

void Demux::Clear()
{
	quitSignal = true;
	while (isInOpenFunc)
	{
		Tools::Get()->Sleep(1);
	}
	mux.lock();

	if (afc)
	{
		//释放并置零 内部会调用avformat_free_context
		avformat_close_input(&afc);
		//avformat_free_context(afc);
		afc = NULL;
	}

	if (avio)
	{
		av_free(avio->buffer);
		//释放并置零
		avio_context_free(&avio);
		avio = NULL;
	}

	if (dataCache)
		dataCache->Clear();

	mux.unlock();

	audioStreamIndex = -1;
	videoStreamIndex = -1;
}

//析构调用
void Demux::Close()
{

	Clear();
	mux.lock();
	delete url;
	url = NULL;

	delete dataCache;
	dataCache = NULL;

	mux.unlock();
}

//多线程安全
bool Demux::Open(char* url)
{
	isInOpenFunc = true;
	if (demuxed)
	{
		cout << "已经解放装成功" << endl;
		isInOpenFunc = false;
		return false;
	}

	if (isDemuxing)
	{
		isInOpenFunc = false;
		StreamDecoder::Get()->PushLog2Net(Warning, "Demuxing please wait!");
		return false;
	}
	//进入到解封装
	isDemuxing = true;
	quitSignal = false;
	isInterruptRead = false;
	dataCache->Clear();

	mux.lock();
	if (afc)
	{
		mux.unlock();
		cout << "严重错误 afc 存在" << endl;
		isDemuxing = false;
		isInOpenFunc = false;
		return false;
	}
	afc = avformat_alloc_context();
	if (!afc)
	{
		mux.unlock();
		StreamDecoder::Get()->PushLog2Net(Warning, "avformat_alloc_context failed!");
		isDemuxing = false;
		isInOpenFunc = false;
		return false;
	}
	startTime = av_gettime();
	bool isSuccess = true;

	//bit 流
	if (url == NULL)
	{
		memset(this->url, 0, URL_LENGTH);

		isSuccess = ProbeInputBuffer();

	}
	//网络流
	else
	{
		memset(this->url, 0, URL_LENGTH);
		memcpy(this->url, url, strlen(url));
		afc->interrupt_callback.opaque = this;
		afc->interrupt_callback.callback = interrupt_cb;
		StreamDecoder::Get()->PushLog2Net(Info, this->url);

		isSuccess = BeginDemux();
	}

	mux.unlock();

	//尽量再Clear前，函数结尾设置IsInOpenFunc 和 isDemuxing
	//Clear 会等待isInOpenFunc
	isDemuxing = false;
	isInOpenFunc = false;
	if (isSuccess)
	{
		if (quitSignal)
		{
			cout << "手动停止" << endl;
			isSuccess = false;
			Clear();
		}
		else
		{
			DemuxSuccess();
		}
	}
	else
	{
		Clear();
	}

	return isSuccess;
}



bool Demux::PushStream2Cache(char* data, int len)
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

void Demux::Start()
{
	//确保decode已经存在并且已经打开成功
	if (!demuxed)
	{
		cout << "Need demux!" << endl;
		return;
	}
	if (isInReadAVPacketFunc)
	{
		cout << "Decoder is run, please wait!" << endl;
		return;
	}
	if (isInterruptRead)
	{
		cout << "read AVPacket is interrupt, please redemux" << endl;
		return;
	}
	std::thread t(&Demux::ReadAVPacket, this);
	t.detach();
}

int Demux::GetCacheFreeSize()
{
	if (dataCache) return dataCacheSize - dataCache->size();
	return 0;
}

AVCodecParameters* Demux::GetVideoPara()
{
	AVCodecParameters *para = NULL;
	para = avcodec_parameters_alloc();
	if (videoStreamIndex < 0)
		return para;

	mux.lock();
	avcodec_parameters_copy(para, afc->streams[videoStreamIndex]->codecpar);
	mux.unlock();
	return para;
}

AVCodecParameters* Demux::GetAudioPara()
{
	AVCodecParameters *para = NULL;
	para = avcodec_parameters_alloc();
	if (audioStreamIndex < 0)
		return para;

	mux.lock();
	avcodec_parameters_copy(para, afc->streams[audioStreamIndex]->codecpar);
	mux.unlock();
	return para;
}

int Demux::interrupt_cb(void* opaque)
{
	Demux* demux = (Demux*)opaque;
	if (!demux) return 1;
	if (demux->quitSignal) return 1;
	return 0;
}

int Demux::read_packet(void *opaque, uint8_t *buf, int buf_size)
{
	Demux* demux = (Demux*)opaque;
	if (!demux) return 0;
	if (demux->quitSignal) return 0;
	size_t lastT = av_gettime();
	//当没有数据等待
	while (demux->dataCache->size() <= 0)
	{
		if (demux->quitSignal) return 0;
		//等待
		Tools::Get()->Sleep(1);

		//处于解封装
		if (demux->isDemuxing)
		{
			if (av_gettime() - demux->startTime > demux->session->config->demuxTimeout * 1000)
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
		//处于av_read_frame
		else
		{
			if (demux->session->config->alwaysWaitBitStream)
			{
				continue;
			}
			else
			{
				//超时读不到数据认为流中断
				if (av_gettime() - lastT > demux->session->config->waitBitStreamTimeout * 1000)
				{
					return 0;
				}
				else
				{
					continue;
				}
			}
		}
	}

	demux->dataCacheMux.lock();
	int size = buf_size < demux->dataCache->size() ? buf_size : demux->dataCache->size();
	memcpy(buf, demux->dataCache->arr, size);
	demux->dataCache->pop_front(size);
	demux->dataCacheMux.unlock();

	return size;
}
bool Demux::ProbeInputBuffer()
{


	//探测流格式  
	//TODO要不要释放？
	AVInputFormat *piFmt = NULL;
	//AVInputFormat* in_fmt = av_find_input_format("h265");

	unsigned char* readBuff = (unsigned char*)av_malloc(BUFF_SIZE);

	if (avio)
	{
		cout << "严重错误 avio 存在" << endl;

		return false;
	}
	//创建 AVIOContext， 使用avio_context_free()释放并置零, readBuff释放用av_free(avio->buffer) 不要定义全局的变量存储，释放全局的会出错
	avio = avio_alloc_context(readBuff, BUFF_SIZE, 0, this, read_packet, NULL, NULL);
	if (!avio)
	{
		StreamDecoder::Get()->PushLog2Net(Warning, "avio_alloc_context failed!");

		return false;
	}
	afc->pb = avio;

	//The caller has supplied a custom AVIOContext, don't avio_close() it.
	//AVFMT_FLAG_CUSTOM_IO
	afc->flags = AVFMT_FLAG_CUSTOM_IO;

	int ret = av_probe_input_buffer(avio, &piFmt, NULL, NULL, 0, 0);

	if (ret < 0)
	{
		StreamDecoder::Get()->PushLog2Net(Warning, Tools::Get()->av_strerror2(ret));

		return false;
	}
	//打印流信息
	char info[100];
	sprintf(info, "Probe stream info success! format:%s[%s]", piFmt->name, piFmt->long_name);
	StreamDecoder::Get()->PushLog2Net(Info, info);
	return BeginDemux();
}

bool Demux::BeginDemux()
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
		StreamDecoder::Get()->PushLog2Net(Warning, "avformat_open_input failed!");
		return false;
	}
	//cout << "avformat_open_input open Success" << endl;
	StreamDecoder::Get()->PushLog2Net(Warning, "avformat_open_input open Success");
	//读取一段视频 获取流信息
	ret = avformat_find_stream_info(afc, NULL);
	StreamDecoder::Get()->PushLog2Net(Warning, "avformat_find_stream_info Success");

	if (ret < 0)
	{
		StreamDecoder::Get()->PushLog2Net(Warning, Tools::Get()->av_strerror2(ret));
		return false;
	}

	//打印流详细信息
	av_dump_format(afc, 0, NULL, 0);


	for (int i = 0; i < afc->nb_streams; i++)
	{
		if (afc->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_UNKNOWN)
		{
			cout << "Stream index[" << i << "]:" << "AVMEDIA_TYPE_UNKNOWN" << endl;
		}
		else if (afc->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			cout << "Stream index[" << i << "]:" << "AVMEDIA_TYPE_VIDEO" << endl;
			videoStreamIndex = i;
		}
		else if (afc->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			cout << "Stream index[" << i << "]:" << "AVMEDIA_TYPE_AUDIO" << endl;
			audioStreamIndex = i;
		}
		else if (afc->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_DATA)
		{
			cout << "Stream index[" << i << "]:" << "AVMEDIA_TYPE_DATA" << endl;
		}
		else if (afc->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE)
		{
			cout << "Stream index[" << i << "]:" << "AVMEDIA_TYPE_SUBTITLE" << endl;
		}
		else if (afc->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_ATTACHMENT)
		{
			cout << "Stream index[" << i << "]:" << "AVMEDIA_TYPE_ATTACHMENT" << endl;
		}
		else if (afc->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_NB)
		{
			cout << "Stream index[" << i << "]:" << "AVMEDIA_TYPE_NB" << endl;
		}
	}
	videoStreamIndex = av_find_best_stream(afc, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);

	return true;
}

void Demux::DemuxSuccess()
{
	session->DemuxSuccess(afc->streams[videoStreamIndex]->codecpar->width, afc->streams[videoStreamIndex]->codecpar->height);
	//解封装成功
	demuxed = true;
	if(session->config->autoDecode) Start();
}

void Demux::ReadAVPacket()
{
	int frame = 0;
	cout << "开始读取帧数据" << endl;
	isInReadAVPacketFunc = true;
	while (!quitSignal)
	{

		mux.lock();
		if (!afc)
		{
			mux.unlock();
			break;
		}


		AVPacket* pkt = av_packet_alloc();

		int ret = av_read_frame(afc, pkt);
		//cout << "[" << ++frame << "]";
		if (ret != 0)
		{
			mux.unlock();
			//读取到结尾
			cout << "read end!!" << endl;
			av_packet_free(&pkt);

			StreamDecoder::Get()->PushLog2Net(Warning, "Stream is interrupt!");
			break;
		}
		mux.unlock();
		//读取一帧数据

		//包含音频
		if (audioStreamIndex != -1)
		{
			if (pkt->stream_index == videoStreamIndex)
			{
				//读取到一帧视频
				session->OnReadOneAVPacket(pkt, false);
			}
			else if (pkt->stream_index == audioStreamIndex)
			{
				//读取到一帧音频
				session->OnReadOneAVPacket(pkt, true);
			}
			else
			{
				av_packet_free(&pkt);
			}
		}
		//不包含音频
		else
		{
			if (pkt->stream_index == videoStreamIndex)
			{
				//读取到一帧视频
				session->OnReadOneAVPacket(pkt, false);
			}
			else
			{
				av_packet_free(&pkt);
			}
		}
		Tools::Get()->Sleep(1);
	}
	isInReadAVPacketFunc = false;
	isInterruptRead = true;
	cout << "结束读取帧数据" << endl;
}
