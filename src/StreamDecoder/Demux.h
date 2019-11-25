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

	//多线程安全
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
	bool isInReadAVPacketFunc = false;
	//标记流是否中断，中断需要重新解封装
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

