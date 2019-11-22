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
			Tools::Get()->Sleep(1);
			if (av_gettime() - session->startTime > session->demuxTimeout * 1000)
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
			if (session->waitQuitSignal)
			{
				return 0;
			}
			Tools::Get()->Sleep(1);
			if (session->alwaysWaitBitStream)
			{
				continue;
			}
			else
			{
				//超时读不到数据认为流中断
				if (av_gettime() - lastT > session->waitBitStreamTimeout * 1000)
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

	session->dataCacheMux.lock();
	int size = bufSize < session->dataCache->size() ? bufSize : session->dataCache->size();
	memcpy(buf, session->dataCache->arr, size);
	session->dataCache->pop_front(size);
	session->dataCacheMux.unlock();

	return size;
}

Session::Session(int playerID, int dataCacheSize)
{
	this->playerID = playerID;
	if (dataCacheSize <= 100000) dataCacheSize = 100000;
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

//用户手动会调用
void Session::Clear()
{
	waitQuitSignal = true;
	mux.lock();
	
	if (isExit)
	{
		mux.unlock();
		return;
	}
	isExit = true;

	while (isDemuxing)
		Tools::Get()->Sleep(1);

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
	if (session->isDemuxing)
	{
		if (av_gettime() - session->startTime > session->demuxTimeout * 1000)
		{
			return 1;
		}
	}
	//cout << "interrupt_cb" << endl;
	return 0;
}

//尝试打开bit流解封装线程
bool Session::TryBitStreamDemux()
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
	isExit = false;
	isRuning = true;
	isInterruptRead = false;
	waitQuitSignal = false;
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
	isDemuxing = true;
	startTime = av_gettime();
	return true;
}

void Session::ProbeInputBuffer()
{

	//探测流格式  
	//TODO要不要释放？
	AVInputFormat *piFmt = NULL;
	//AVInputFormat* in_fmt = av_find_input_format("h265");

	unsigned char* readBuff = (unsigned char*)av_malloc(BUFF_SIZE);

	//创建 AVIOContext， 使用avio_context_free()释放并置零, readBuff释放用av_free(avio->buffer) 不要定义全局的变量存储，释放全局的会出错
	if (avio)
	{
		
		cout << "严重错误 avio 存在" << endl;
		isRuning = false;
		isDemuxing = false;
		Clear();
		return;
	}
	avio = avio_alloc_context(readBuff, BUFF_SIZE, 0, this, ReadPacket, NULL, NULL);
	if (!avio)
	{
		
		StreamDecoder::Get()->PushLog2Net(Warning, "avio_alloc_context failed!");
		isRuning = false;
		isDemuxing = false;
		Clear();
		return;
	}
	afc->pb = avio;
	
	//The caller has supplied a custom AVIOContext, don't avio_close() it.
	//AVFMT_FLAG_CUSTOM_IO
	afc->flags = AVFMT_FLAG_CUSTOM_IO;

	//isProbeBuffer = true;
	int ret = av_probe_input_buffer(avio, &piFmt, NULL, NULL, 0, 0);
	//isProbeBuffer = false;
	if (ret < 0)
	{
		
		StreamDecoder::Get()->PushLog2Net(Warning, Tools::Get()->av_strerror2(ret));
		isRuning = false;
		isDemuxing = false;
		Clear();
		return;
	}

	char info[100];
	sprintf(info, "format:%s[%s]", piFmt->name, piFmt->long_name);
	StreamDecoder::Get()->PushLog2Net(Info, info);

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
		
		StreamDecoder::Get()->PushLog2Net(Warning, "avformat_open_input failed!");
		isRuning = false;
		isDemuxing = false;
		Clear();
		return;
	}
	cout << "avformat_open_input open Success" << endl;
	//读取一段视频 获取流信息
	ret = avformat_find_stream_info(afc, NULL);
	if (ret < 0)
	{
		
		StreamDecoder::Get()->PushLog2Net(Warning, Tools::Get()->av_strerror2(ret));
		isRuning = false;
		isDemuxing = false;
		Clear();
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
	if (width > height) isLandscape = true;
	if (decode)
	{
	
		cout << "严重错误 decode 存在" << endl;
		isRuning = false;
		isDemuxing = false;
		Clear();
	}
	decode = new Decode(this);
	if (decode && !decode->Open(afc->streams[videoStreamIndex]->codecpar))
	{
	
		StreamDecoder::Get()->PushLog2Net(Warning, "open decode failed!");
		isRuning = false;
		isDemuxing = false;
		Clear();
		return;
	}

	isDemuxing = false;
	StreamDecoder::Get()->PushLog2Net(Info, "Demux Success!");

}

void Session::BeginDecode()
{
	
	//正在解封装
	if (isDemuxing)
	{
		StreamDecoder::Get()->PushLog2Net(Warning, "Please wait demuxing!");
		return;
	}
	//没有正确解封装
	if (!isRuning)
	{
		StreamDecoder::Get()->PushLog2Net(Warning, "Need demux!");
		return;
	}

	if (decode)
	{
		if (decode->isRuning)
		{
			StreamDecoder::Get()->PushLog2Net(Warning, "Decoder thread is runing!");
		}
		else
		{
			StreamDecoder::Get()->PushLog2Net(Warning, "Begin decode thread success!");
			std::thread decode_t(&Decode::run, decode);
			decode_t.detach();
			
		}
	}
	
	if (isInReadPacketThread)
	{
		StreamDecoder::Get()->PushLog2Net(Warning, "Read packet thread is runing!");
		return;
	}

	if (isInterruptRead)
	{
		StreamDecoder::Get()->PushLog2Net(Warning, "Read packet thread is interrupt!");
	}
	else
	{
		StreamDecoder::Get()->PushLog2Net(Warning, "Begin read packet thread!");
		std::thread t(&Session::run, this);
		t.detach();
	}
}

void Session::StopDecode()
{
	if (!isRuning)
	{
		StreamDecoder::Get()->PushLog2Net(Warning, "Don't need stop decode!");
		return;
	}
	StreamDecoder::Get()->PushLog2Net(Warning, "Stop decode!");
	Clear();
}



int Session::GetCacheFreeSize()
{
	return dataCacheSize - dataCache->size();
}

void Session::OnDecodeOnFrame(AVFrame *frame)
{
	if (isExit)
	{
		av_frame_free(&frame);
		return;
	}

	int width = frame->width;
	int height = frame->height;
	bool landscape = width > height ? true : false;
	if (landscape != isLandscape)
	{
		//屏幕反转了
		width = this->height;
		height = this->width;
	}

	//不需要内存对齐
	if (frame->linesize[0] == width)
	{
		Frame *tmpFrame = new Frame(playerID, frame->width, frame->height, (char*)frame->data[0], (char*)frame->data[1], (char*)frame->data[2]);
		StreamDecoder::Get()->PushFrame2Net(tmpFrame);
	}
	else
	{
		Frame *tmpFrame = new Frame(playerID, width, height, NULL, NULL, NULL, false);
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
	av_frame_free(&frame);

	//会造成Session::run的阻塞
	if (pushFrameInterval > 0)
	{
		Tools::Get()->Sleep(pushFrameInterval);
	}
}



void Session::SetOption(int optionType, int value)
{
	//安全校验
	if (value < 0) value = 0;

	if ((OptionType)optionType == OptionType::DemuxTimeout)
	{
		demuxTimeout = value;
	}
	else if((OptionType)optionType == OptionType::PushFrameInterval)
	{
		pushFrameInterval = value;
	}
	else if ((OptionType)optionType == OptionType::AlwaysWaitBitStream)
	{
		alwaysWaitBitStream = value == 0 ? false : true;
	}
	else if ((OptionType)optionType == OptionType::WaitBitStreamTimeout)
	{
		waitBitStreamTimeout = value;
	}
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
		int ret = 0;
		if (!afc)
		{
			cout << "afc 已经释放" << endl;
			mux.unlock();
			break;
		}
		ret = av_read_frame(afc, pkt);
		//cout << "read a frame" << endl;
		if (ret != 0)
		{
			mux.unlock();
			//读取到结尾
			cout << "read end!!" << endl;
			av_packet_free(&pkt);
			isInterruptRead = true;
			//cout << Tools::Get()->av_strerror2(ret) << endl;
			StreamDecoder::Get()->PushLog2Net(Warning, Tools::Get()->av_strerror2(ret));
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