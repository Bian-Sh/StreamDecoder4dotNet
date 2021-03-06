#include "Session.h"
#include "Decode.h"
#include "StreamDecoder.h"
#include "Packet.h"
#include <iostream>
#include "Tools.h"
#include "Demux.h"
#include <windows.h>

#define USE_LIBYUV


#ifdef USE_LIBYUV
#include <libyuv.h>
#else
extern "C" {
#include "ConvertYUV.h"
}
#endif


using namespace std;
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libavutil/time.h>
}


void _stdcall SessionTimerProcess(HWND hwnd, UINT uMsg, UINT_PTR timerPtr, DWORD dwTime)
{
	
}

Session::Session()
	:verifyValue(VERIFY_VALUE)
{
	config = new SessionConfig();

#ifndef USE_LIBYUV
	InitConverter();
#endif

}

Session::~Session()
{
	verifyValue = 0;
	Close();
	cout << "~Session" << endl;
}
//析构函数调用
void Session::Close()
{
	Clear();

	funcMux.lock();
	this->opaque = NULL;
	DotNetSessionEvent = NULL;
	DotNetDrawFrame = NULL;
	funcMux.unlock();

	mux.lock();
	if (config)
	{
		delete config;
		config = NULL;
	}
	mux.unlock();
}

//用户手动会调用
void Session::Clear()
{
	quitSignal = true;
	mux.lock();

	if (demux)
	{
		delete demux;
		demux = NULL;
	}

	if (vdecode)
	{
		delete vdecode;
		vdecode = NULL;
	}
	//清空列表
	Update(false);
	mux.unlock();
}

//添加数据流
bool Session::PushStream2Cache(char* data, int len)
{
	mux.lock();
	if (!demux)
	{
		mux.unlock();
		return false;
	}
	bool b = demux->PushStream2Cache(data, len);
	mux.unlock();
	return b;
}


//尝试打开网络流解封装线程
void Session::TryStreamDemux(char* url)
{
	quitSignal = false;
	if (!demux) demux = new Demux(this);

	if (url != NULL)
	{
		this->url = new char[512];
		memset(this->url, 0, 512);
		memcpy(this->url, url, strlen(url));
	}
	std::thread t(&Session::OpenDemuxThread, this);
	t.detach();
}



void Session::OpenDemuxThread()
{
	StreamDecoder::Get()->PushLog2Net(Info, "Try open stream!");
	cout << "解封装结果:" << (demux->Open(url) == false ? "false" : "true") << endl;
	if (url)
	{
		delete url;
		url = NULL;
	}
}



void Session::BeginDecode()
{
	mux.lock();
	if (!demux)
	{
		mux.unlock();
		cout << "解封装器不存在" << endl;
		return;
	}
	if (!vdecode)
	{
		mux.unlock();
		cout << "解码器不存在" << endl;
		return;
	}
	if (!vdecode->isOpened)
	{
		mux.unlock();
		cout << "解码器未打开" << endl;
		return;
	}
	mux.unlock();
	demux->Start();
}

void Session::EndDecode()
{
	Clear();
}



int Session::GetCacheFreeSize()
{
	if (demux) return demux->GetCacheFreeSize();
	return 0;
}

void Session::DemuxSuccess(int width, int height)
{
	this->width = width;
	this->height = height;
	//视频为横向
	if (width > height) isLandscape = true;
	mux.lock();
	if (vdecode)
	{
		mux.unlock();
		cout << "严重错误 解码器已经存在" << endl;
		return;
	}
	if (!vdecode) vdecode = new Decode(this, false);
	if (!vdecode->Open(demux->GetVideoPara()))
	{
		cout << "严重错误 解码器没有打开" << endl;
		mux.unlock();
		return;
	}
	vdecode->Start();

	funcMux.lock();
	eventPackets.push_back(SessionEventType::DemuxSuccess);
	funcMux.unlock();
	mux.unlock();
}

void Session::OnReadOneAVPacket(AVPacket* packet, bool isAudio)
{
	if (quitSignal)
	{
		av_packet_free(&packet);
		return;
	}
	if (isAudio)
	{
		av_packet_free(&packet);
		return;
	}

	while (true)
	{
		if (quitSignal)
		{
			av_packet_free(&packet);
			break;
		}
		mux.lock();
		bool b = vdecode->Push(packet);
		mux.unlock();

		if (b) break;
		Tools::Get()->Sleep(1);
		continue;
	}

}

void Session::OnDecodeOneAVFrame(AVFrame *frame, bool isAudio)
{
	
	if (quitSignal)
	{
		av_frame_free(&frame);
		return;
	}
	if (isAudio)
	{
		av_frame_free(&frame);
		return;
	}

	//初始AVFrame
	int width = frame->width;
	int height = frame->height;
	bool landscape = width > height ? true : false;
	if (landscape != isLandscape)
	{
		//屏幕反转了
		width = this->height;
		height = this->width;
	}

	Frame * tmpFrame = NULL;
	//不需要内存对齐
	if (frame->linesize[0] == width)
	{
		tmpFrame = new Frame(
			//config->playerID, 
			frame->pkt_dts, 
			Tools::Get()->GetTimestamp(),
			frame->pts,
			demux->fps,
			width, 
			height, 
			(char*)frame->data[0], 
			(char*)frame->data[1], 
			(char*)frame->data[2]);
	}
	//需要行对齐
	else
	{
		tmpFrame = new Frame(
			//config->playerID, 
			frame->pkt_dts,
			Tools::Get()->GetTimestamp(), 
			frame->pts,
			demux->fps,
			width, 
			height, 
			NULL, NULL, NULL, 
			false);
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
	}
	av_frame_free(&frame);

	if (config->useCPUConvertYUV)
	{
		tmpFrame->rgba = new char[width * height * 4];
		
		
#ifdef USE_LIBYUV
		if (config->convertPixelFormat == RGBA)
		{
			//内存中存储方式为RGBA
			libyuv::I420ToABGR(
				(uint8_t*)tmpFrame->frame_y, width,
				(uint8_t*)tmpFrame->frame_u, width / 2,
				(uint8_t*)tmpFrame->frame_v, width / 2, (uint8_t*)tmpFrame->rgba, width * 4, width, height);
		}
		else if (config->convertPixelFormat == BGRA)
		{
			libyuv::I420ToARGB(
				(uint8_t*)tmpFrame->frame_y, width,
				(uint8_t*)tmpFrame->frame_u, width / 2,
				(uint8_t*)tmpFrame->frame_v, width / 2, (uint8_t*)tmpFrame->rgba, width * 4, width, height);
		}

#else

		if (config->convertPixelFormat == RGBA)
		{
			I420toRGBA((unsigned char*)tmpFrame->frame_y, (unsigned char*)tmpFrame->frame_u, (unsigned char*)tmpFrame->frame_v, width, height, (unsigned char*)tmpFrame->rgb);
		}
		else if (config->convertPixelFormat == BGRA)
		{
			I420toBGRA((unsigned char*)tmpFrame->frame_y, (unsigned char*)tmpFrame->frame_u, (unsigned char*)tmpFrame->frame_v, width, height, (unsigned char*)tmpFrame->rgb);
		}

#endif
		
	}

	
	tmpFrame->bsdnts = Tools::Get()->GetTimestamp();
	//cout << tmpFrame->pts << endl;
	if (config->asyncUpdate)
	{
		//cout << "demux" << (demux == NULL)<<endl;
		//mux.lock();

		//if (!quitSignal && DotNetDrawFrame)
		//{
		//	//异步直接调用
		//	DotNetDrawFrame(opaque, tmpFrame);
		//}
		//
		//mux.unlock();
		if (DotNetDrawFrame) DotNetDrawFrame(opaque, tmpFrame);
		delete tmpFrame;
		tmpFrame = NULL;
	}
	else
	{
		//同步到主线程调用
		funcMux.lock();
		framePackets.push_back(tmpFrame);
		funcMux.unlock();
	}
	

	if (config->pushFrameInterval > 0)
	{
		Tools::Get()->Sleep(config->pushFrameInterval);
	}
}


//主线程调用
//call_cb是否调用回调函数
void Session::Update(bool call_cb)
{
	if (framePackets.size() == 0 && eventPackets.size() == 0) return;

	funcMux.lock();
	int size = framePackets.size();
	for (int i = 0; i < size; i++)
	{
		Frame* frame = framePackets.front();
		framePackets.pop_front();
		if (call_cb && DotNetDrawFrame) DotNetDrawFrame(opaque, frame);
		delete frame;
		frame = NULL;
	}
	
	size = eventPackets.size();
	for (int i = 0; i < size; i++)
	{
		int eventType = eventPackets.front();
		eventPackets.pop_front();
		if (call_cb && DotNetSessionEvent) DotNetSessionEvent(opaque, eventType);
	}
	funcMux.unlock();
}

void Session::SetOption(int optionType, int value)
{
	//安全校验
	if (value < 0) value = 0;

	if ((OptionType)optionType == OptionType::DataCacheSize)
	{
		//最小500K
		if (value < 500000) value = 500000;
		config->dataCacheSize = value;
	}
	else if ((OptionType)optionType == OptionType::DemuxTimeout)
	{
		config->demuxTimeout = value;
	}
	else if ((OptionType)optionType == OptionType::PushFrameInterval)
	{
		config->pushFrameInterval = value;
	}
	else if ((OptionType)optionType == OptionType::AlwaysWaitBitStream)
	{
		//0为false 其余为true
		config->alwaysWaitBitStream = value;
	}
	else if ((OptionType)optionType == OptionType::WaitBitStreamTimeout)
	{
		config->waitBitStreamTimeout = value;
	}
	else if ((OptionType)optionType == OptionType::AutoDecode)
	{
		//0为false 其余为true
		config->autoDecode = value;
	}
	else if ((OptionType)optionType == OptionType::DecodeThreadCount)
	{
		if (value > 8) value = 8;
		config->decodeThreadCount = value;
	}
	else if ((OptionType)optionType == OptionType::UseCPUConvertYUV)
	{
		config->useCPUConvertYUV = value;
	}
	else if ((OptionType)optionType == OptionType::ConvertPixelFormat)
	{
		config->convertPixelFormat = value;
	}
	else if ((OptionType)optionType == OptionType::AsyncUpdate)
	{
		config->asyncUpdate = value;
	}
}

void Session::SetEventCallBack(PEvent pEvent, PDrawFrame pDrawFrame, void* opaque)
{
	funcMux.lock();
	this->opaque = opaque;
	DotNetSessionEvent = pEvent;
	DotNetDrawFrame = pDrawFrame;
	funcMux.unlock();
}


