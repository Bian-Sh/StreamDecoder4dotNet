#include "Decode.h"
#include "Session.h"
#include "Tools.h"
#include <iostream>
using namespace std;
extern "C"
{
#include <libavcodec/avcodec.h>
}
#define  CODEC_THREAD_COUNT 4
Decode::Decode(Session *session)
{
	this->session = session;
}

Decode::~Decode()
{
	Close();
}

bool Decode::Open(AVCodecParameters *para)
{
	if (!para) return false;

	//查找解码器
	//是否需要释放TODO
	AVCodec *avcodec = avcodec_find_decoder(para->codec_id);

	if (!avcodec)
	{
		cout << "can't find video AVCodec: id=" << para->codec_id << endl;
		avcodec_parameters_free(&para);
		return false;
	}
	mux.lock();
	//创建解码器上下文
	codec = avcodec_alloc_context3(avcodec);

	//复制解码器上下文参数
	avcodec_parameters_to_context(codec, para);
	//释放参数
	avcodec_parameters_free(&para);
	//打开解码器
	codec->thread_count = CODEC_THREAD_COUNT;
	int ret = avcodec_open2(codec, NULL, NULL);
	if (ret != 0)
	{
		mux.unlock();
		cout << Tools::Get()->av_strerror2(ret) << endl;
		return false;
	}
	cout << "open codec success!" << endl;
	mux.unlock();
	return true;
}

void Decode::Push(AVPacket *pkt)
{
	while (packets.size() > 10)
	{
		if (isExit) return;
		Tools::Get()->Sleep(1);
		continue;
	}
	mux.lock();
	if (!codec)
	{
		mux.unlock();
		return;
	}
	packets.push_back(pkt);
	mux.unlock();
}

void Decode::Close()
{
	mux.lock();
	if (!codec)
	{
		mux.unlock();
		return;
	}
	isExit = true;
	avcodec_close(codec);
	//释放编解码器上下文和所有与之相关的内容，并写入NULL。
	avcodec_free_context(&codec);
	int size = packets.size();
	for (int i = 0; i < size; i++)
	{
		AVPacket *pkt = packets.front();
		packets.pop_front();
		av_packet_free(&pkt);
	}
	mux.unlock();

	while (isRuning)
		Tools::Get()->Sleep(1);
}

void Decode::run()
{
	isRuning = true;
	while (!isExit)
	{
		int size = packets.size();
		if (size <= 0)
		{
			Tools::Get()->Sleep(1);
			continue;
		}
		mux.lock();
		AVPacket* pkt = packets.front();
		packets.pop_front();
		//发送并解码
		int ret = avcodec_send_packet(codec, pkt);
		av_packet_free(&pkt);
		mux.unlock();

		while (!isExit)
		{
			mux.lock();
			if (!codec) return;
			AVFrame *frame = av_frame_alloc();
			int ret = avcodec_receive_frame(codec, frame);
			mux.unlock();

			if (ret != 0)
			{
				av_frame_free(&frame);
				break;
			}

			//处理解码出来的数据
			if (session)
			{
				session->OnDecodeOneAVFrame(frame);
			}
			Tools::Get()->Sleep(1);
		}
		Tools::Get()->Sleep(1);
	}
	isRuning = false;
}
