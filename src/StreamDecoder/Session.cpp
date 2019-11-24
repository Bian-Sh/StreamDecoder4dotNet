#include "Session.h"
#include "Decode.h"
#include "StreamDecoder.h"
#include "Packet.h"
#include <iostream>
#include "Tools.h"
#include "Demux.h"
using namespace std;
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libavutil/time.h>
}


#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")

#define USE_DECODER


Session::Session(int playerID, int cacheSize)
{
	this->playerID = playerID;
	dataCacheSize = cacheSize;
	//demux = new Demux(dataCacheSize, demuxTimeout, alwaysWaitBitStream, waitBitStreamTimeout);
}

Session::~Session()
{
	Close();
	cout << "~Session" << endl;
}
void Session::Close()
{
	Clear();
}

//用户手动会调用
void Session::Clear()
{

	mux.lock();
	
	if (demux)
	{
		delete demux;
		demux = NULL;
	}
	mux.unlock();
}

//添加数据流
bool Session::PushStream2Cache(char* data, int len)
{
	if (!demux) return false;
	return demux->PushStream2Cache(data, len);
}


void Session::OnReadOneAVPacket(AVPacket* packet, bool isAudio)
{
	av_packet_free(&packet);
}

//尝试打开网络流解封装线程
void Session::TryStreamDemux(char* url)
{
	if(!demux) demux = new Demux(this, dataCacheSize, demuxTimeout, alwaysWaitBitStream, waitBitStreamTimeout);
	std::thread t(&Session::OpenDemuxThread, this, url);
	t.detach();
}



void Session::OpenDemuxThread(char* url)
{
	cout << "解封装结果:" << (demux->Open(url) == false ? "false" : "true") << endl;
}



void Session::BeginDecode()
{
	if (!demux)
	{
		cout << "解封装器不存在" << endl;
		return;
	}
	demux->Start();
}

void Session::StopDecode()
{
	Clear();
}



int Session::GetCacheFreeSize()
{
	if (demux) return demux->GetCacheFreeSize();
	return 0;
}

void Session::OnDecodeOneAVFrame(AVFrame *frame)
{
	/*if (isExit)
	{
		av_frame_free(&frame);
		return;
	}*/

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
	else if ((OptionType)optionType == OptionType::PushFrameInterval)
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

}