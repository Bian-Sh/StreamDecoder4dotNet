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

	//���۴�����ͷ� AVCodecParameters
	bool Open(AVCodecParameters *para);

	bool Push(AVPacket *pkt);

	void Start();
public:

	bool isOpened = false;

private:
	void DecodeAVPacket();
	//��������������ģ����رգ�ָ����ΪNULL
	void Close();

private:
	bool isInDecodeAVPacketFunc = false;
	std::mutex mux;

	//�����������ģ���Ҫ�ͷ� avcodec_free_context
	AVCodecContext* codec = NULL;

	//��Ҫ����
	std::list<AVPacket*> packets;

	bool quitSignal = false;

	Session* session = NULL;

	bool isAudioDecodec = false;
};
