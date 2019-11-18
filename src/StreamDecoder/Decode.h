#pragma once
#include <mutex>
#include <list>
class Decode
{

public:
	Decode(class Session *session);
	~Decode();

	//无论打开与否都释放 AVCodecParameters
	bool Open(struct AVCodecParameters *para);

	void Push(struct AVPacket *pkt);

	//清理解码器上下文，并关闭，指针置为NULL
	void Close();

	void run();

private:
	std::mutex mux;

	//解码器上下文，需要释放 avcodec_free_context
	struct AVCodecContext* codec = NULL;

	//需要清理
	std::list<AVPacket*> packets;

	bool isExit = false;

	Session* session = NULL;
};
