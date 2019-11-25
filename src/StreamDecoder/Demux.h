#pragma once
#include <mutex>
struct AVIOContext;
struct AVFormatContext;
struct AVCodecParameters;
struct SessionConfig;
class SCharList;
class Session;
class Demux
{
public:
	Demux(Session* session);
	~Demux();

	//���̰߳�ȫ
	bool Open(char* url = NULL);

	bool PushStream2Cache(char* data, int len);

	void Start();

	int GetCacheFreeSize();

	AVCodecParameters* GetVideoPara();
	AVCodecParameters* GetAudioPara();
public:

	SCharList *dataCache = NULL;
	std::mutex dataCacheMux;

	bool quitSignal = false;

private:
	static int interrupt_cb(void* opaque);
	static int read_packet(void *opaque, uint8_t *buf, int buf_size);

	void Close();
	void Clear();

	bool ProbeInputBuffer();

	bool BeginDemux();

	void DemuxSuccess();

	void ReadAVPacket();

private:
	//��Ҫ����
	AVIOContext* avio = NULL;
	//��Ҫ����
	AVFormatContext* afc = NULL;

	int64_t startTime = 0;

	//��Ҫ�ͷ�
	char* url = NULL;

	std::mutex mux;

	//���������Ľ��װ��
	bool isDemuxing = false;
	//�Ѿ����װ�ɹ�
	bool demuxed = false;
	//��isDemuxing����һ�£��������ڲ�ͬ
	bool isInOpenFunc = false;
	bool isInReadAVPacketFunc = false;
	//������Ƿ��жϣ��ж���Ҫ���½��װ
	bool isInterruptRead = false;

	int dataCacheSize = 1000000;

	Session* session = NULL;

	int videoStreamIndex = -1;
	int audioStreamIndex = -1;
};
//url
//avio
//afc
//dataCache

