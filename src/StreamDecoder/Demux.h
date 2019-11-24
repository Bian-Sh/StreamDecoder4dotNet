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

	//多线程安全
	bool Open(char* url = NULL);

	bool PushStream2Cache(char* data, int len);

public:

	SCharList *dataCache = NULL;
	std::mutex dataCacheMux;

	//解封装等待时间
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
	//需要清理
	AVIOContext* avio = NULL;
	//需要清理
	AVFormatContext* afc = NULL;

	int64_t startTime = 0;

	//需要释放
	char* url = NULL;

	std::mutex mux;

	//处于真正的解封装中
	bool isDemuxing = false;
	//已经解封装成功
	bool demuxed = false;
	//和isDemuxing基本一致，生命周期不同
	bool isInOpenFunc = false;

	int dataCacheSize = 1000000;
};
//url
//avio
//afc
//dataCache

