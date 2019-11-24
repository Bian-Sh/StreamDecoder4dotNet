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

	//���ҽ�����
	//�Ƿ���Ҫ�ͷ�TODO
	AVCodec *avcodec = avcodec_find_decoder(para->codec_id);

	if (!avcodec)
	{
		cout << "can't find video AVCodec: id=" << para->codec_id << endl;
		avcodec_parameters_free(&para);
		return false;
	}
	mux.lock();
	//����������������
	codec = avcodec_alloc_context3(avcodec);

	//���ƽ����������Ĳ���
	avcodec_parameters_to_context(codec, para);
	//�ͷŲ���
	avcodec_parameters_free(&para);
	//�򿪽�����
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
	//�ͷű�����������ĺ�������֮��ص����ݣ���д��NULL��
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
		//���Ͳ�����
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

			//����������������
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
