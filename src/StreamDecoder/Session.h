#pragma once

#include <mutex>
class Decode;

//毫秒级别的sleep
void Sleep(int ms);

class Session
{

public:
	Session(int dataCacheSize);
	~Session();

	bool TryDemux(int waitDemuxTime);
	bool TryNetStreamDemux(char* url);

	void BeginDecode();
	//
	void StopDecode();

	void Close();
	//添加数据流
	int GetCacheFreeSize();
	bool PushStream2Cache(char* data, int len);

	static char* av_strerror2(int errnum);

	void OnDecodeOnFrame(struct AVFrame *frame);
	
	//数据缓冲区大小
	int dataCacheSize = 500000;
	//删除时候清理
	class SCharList *dataCache = NULL;
	std::mutex dataCacheMux;

	//退出信号，true 退出线程
	bool isExit = false;

	std::mutex mux;
	

	//解封装等待时间
	int waitDemuxTime = 2000;
	//是否处于Demux状态 重要的作用在ReadPacket的回调函数中
	bool isDemuxing = false;

	//解封装前的时间戳ns
	int64_t startTime = 0;
	

//signals:
//	void OnDemuxSuccessSignal(bool isSuccess, char* info);

private:
	void ProbeInputBuffer();
	bool OpenDemuxThread();
	void Demux();
	//线程函数
	void run();

	struct AVIOContext* avio = NULL;
	struct AVFormatContext* afc = NULL;

	int videoStreamIndex = -1;
	int audioStreamIndex = -1;
	//QMutex yuvMux;
	
	//需要清理
	Decode *decode = NULL;

	int width = 0;
	int height = 0;

	//需要清理
	/*unsigned char* yuv[3] = {0};
	int linesizeY = 0;*/

	//bool isDecodeing = false;

private:
	//ReadPacket线程是否运行标志位
	bool isInReadPacketThread = false;

	//是否处于解封装、解码的过程（运行过程）
	bool isRuning = false;

	static char* logbuf;

	char* url = NULL;
};
