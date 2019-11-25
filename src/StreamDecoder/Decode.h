#pragma once
#include <mutex>
#include <list>

struct AVCodecContext;
struct AVCodecParameters;
struct AVPacket;
class Session;

class Decode
{

public:
	Decode(Session *session, bool isAudio);
	~Decode();

	//无论打开与否都释放 AVCodecParameters
	bool Open(AVCodecParameters *para);

	bool Push(AVPacket *pkt);

	void Start();
public:

	bool isOpened = false;

private:
	void DecodeAVPacket();
	//清理解码器上下文，并关闭，指针置为NULL
	void Close();

private:
	bool isInDecodeAVPacketFunc = false;
	std::mutex mux;

	//解码器上下文，需要释放 avcodec_free_context
	AVCodecContext* codec = NULL;

	//需要清理
	std::list<AVPacket*> packets;

	bool quitSignal = false;

	Session* session = NULL;

	bool isAudioDecodec = false;
};
