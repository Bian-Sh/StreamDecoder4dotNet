#pragma once
#include <mutex>
#include <list>

struct AVCodecContext;

class Decode
{

public:
	Decode(class Session *session);
	~Decode();

	//���۴�����ͷ� AVCodecParameters
	bool Open(struct AVCodecParameters *para);

	void Push(struct AVPacket *pkt);

	//��������������ģ����رգ�ָ����ΪNULL
	void Close();

	void run();

private:
	std::mutex mux;

	//�����������ģ���Ҫ�ͷ� avcodec_free_context
	AVCodecContext* codec = NULL;

	//��Ҫ����
	std::list<AVPacket*> packets;

	bool isExit = false;

	Session* session = NULL;
};
