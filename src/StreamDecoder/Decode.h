#pragma once
#include <mutex>
#include <list>

struct AVCodecContext;
struct AVCodecParameters;
struct AVPacket;

class Decode
{

public:
	Decode(class Session *session);
	~Decode();

	//无论打开与否都释放 AVCodecParameters
	bool Open(AVCodecParameters *para);

	void Push(AVPacket *pkt);


	void run();

	bool isRuning = false;
	

private:

	//清理解码器上下文，并关闭，指针置为NULL
	void Close();

	std::mutex mux;

	//解码器上下文，需要释放 avcodec_free_context
	AVCodecContext* codec = NULL;

	//需要清理
	std::list<AVPacket*> packets;

	bool isExit = false;

	Session* session = NULL;
};
