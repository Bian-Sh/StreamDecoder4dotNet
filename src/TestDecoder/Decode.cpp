#include "Decode.h"
#include <QDebug>
#include "Session.h"
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
	QMutexLocker locker(&mux);
	packets.clear();
}

//无论打开与否都释放 AVCodecParameters
bool Decode::Open(AVCodecParameters *para)
{
	if (!para) return false;

	//查找解码器
	AVCodec *avcodec = avcodec_find_decoder(para->codec_id);
	if (!avcodec)
	{
		qDebug() << "can't find video AVCodec: id=" << para->codec_id;
		avcodec_parameters_free(&para);
		return false;
	}
	QMutexLocker locker(&mux);
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
		Session::av_strerror2(ret);
		return false;
	}
	qDebug() << "open codec success!";
	return true;
}

void Decode::Push(struct AVPacket *pkt)
{
	while (packets.size() > 10)
	{
		QThread::msleep(1);
		continue;
	}
	QMutexLocker locker(&mux);
	packets.push_back(pkt);

}


void Decode::Close()
{
	QMutexLocker locker(&mux);
	if (!codec) return;
	isExit = true;
	avcodec_close(codec);
	//释放编解码器上下文和所有与之相关的内容，并写入NULL。
	avcodec_free_context(&codec);
}

void Decode::run()
{
	while (!isExit)
	{
		int size = packets.size();
		if (size <= 0)
		{
			QThread::msleep(1);
			continue;
		}
		QMutexLocker locker(&mux);
		AVPacket* pkt = packets.front();
		packets.pop_front();
		//发送并解码
		int ret = avcodec_send_packet(codec, pkt);
		av_packet_free(&pkt);
		locker.unlock();

		while (!isExit)
		{
			locker.relock();
			if (!codec) return;
			AVFrame *frame = av_frame_alloc();
			int ret = avcodec_receive_frame(codec, frame);
			locker.unlock();
			if (ret != 0)
			{
				av_frame_free(&frame);
				break;
			}
			if (session)
			{
				session->OnDecodeOnFrame(frame);
			}
			QThread::msleep(1);
		}
		QThread::msleep(1);
	}
}
