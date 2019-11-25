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

#define USE_DECODER


Session::Session(int playerID, int cacheSize)
{
	config = new SessionConfig();
	config->playerID = playerID;
	config->dataCacheSize = cacheSize;
	//demux = new Demux(dataCacheSize, demuxTimeout, alwaysWaitBitStream, waitBitStreamTimeout);
}

Session::~Session()
{
	Close();
	cout << "~Session" << endl;
}
//析构函数调用
void Session::Close()
{
	Clear();
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
	mux.unlock();
}

//添加数据流
bool Session::PushStream2Cache(char* data, int len)
{
	if (!demux) return false;
	return demux->PushStream2Cache(data, len);
}






//尝试打开网络流解封装线程
void Session::TryStreamDemux(char* url)
{
	quitSignal = false;
	if (!demux) demux = new Demux(this, config);
	std::thread t(&Session::OpenDemuxThread, this, url);
	t.detach();
}



void Session::OpenDemuxThread(char* url)
{
	cout << "解封装结果:" << (demux->Open(url) == false ? "false" : "true") << endl;
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

void Session::StopDecode()
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

	//不需要内存对齐
	if (frame->linesize[0] == width)
	{
		Frame *tmpFrame = new Frame(config->playerID, frame->width, frame->height, (char*)frame->data[0], (char*)frame->data[1], (char*)frame->data[2]);
		StreamDecoder::Get()->PushFrame2Net(tmpFrame);
	}
	else
	{
		Frame *tmpFrame = new Frame(config->playerID, width, height, NULL, NULL, NULL, false);
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
	if (config->pushFrameInterval > 0)
	{
		Tools::Get()->Sleep(config->pushFrameInterval);
	}
}



void Session::SetOption(int optionType, int value)
{
	//安全校验
	if (value < 0) value = 0;

	if ((OptionType)optionType == OptionType::DemuxTimeout)
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
}
