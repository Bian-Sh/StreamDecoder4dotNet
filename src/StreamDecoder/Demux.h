#pragma once
#include <mutex>
struct AVIOContext;
struct AVFormatContext;
class SCharList;
class Demux
{
public:
	Demux(int cacheSize, int demuxTimeout, bool alwaysWaitBitStream, int waitBitStreamTimeout);
	~Demux();

	//���̰߳�ȫ
	bool Open(char* url = NULL);

	bool PushStream2Cache(char* data, int len);

public:

	SCharList *dataCache = NULL;
	std::mutex dataCacheMux;

	//���װ�ȴ�ʱ��
	int demuxTimeout = 2000;
	bool alwaysWaitBitStream = false;
	int waitBitStreamTimeout = 1000;

	bool quitSignal = false;

private:
	static int interrupt_cb(void* opaque);
	static int read_packet(void *opaque, uint8_t *buf, int buf_size);

	void Close();
	void Clear();

	bool ProbeInputBuffer();

	bool BeginDemux();

	void DemuxSuccess();

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

	int dataCacheSize = 1000000;
};
//url
//avio
//afc
//dataCache

