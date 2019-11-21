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

	//���۴�����ͷ� AVCodecParameters
	bool Open(AVCodecParameters *para);

	void Push(AVPacket *pkt);


	void run();

	bool isRuning = false;
	

private:

	//��������������ģ����رգ�ָ����ΪNULL
	void Close();

	std::mutex mux;

	//�����������ģ���Ҫ�ͷ� avcodec_free_context
	AVCodecContext* codec = NULL;

	//��Ҫ����
	std::list<AVPacket*> packets;

	bool isExit = false;

	Session* session = NULL;
};
