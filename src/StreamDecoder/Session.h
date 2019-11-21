#pragma once

#include <mutex>
class Decode;
class SCharList;
struct AVIOContext;
struct AVFormatContext;
struct AVFrame;

//毫秒级别的sleep
void Sleep(int ms);

class Session
{

public:
	Session(int dataCacheSize);
	~Session();

	//打开字节流数据
	bool TryBitStreamDemux(int waitDemuxTime);
	//打开rtmp
	bool TryNetStreamDemux(char* url);
	//开始解码
	void BeginDecode();
	//停止解码
	void StopDecode();
	
	//获取 字节流缓冲 空余空间大小
	int GetCacheFreeSize();
	//添加数据流
	bool PushStream2Cache(char* data, int len);

	//解码完成后Decode调用
	void OnDecodeOnFrame(AVFrame *frame);
	

public:

	//数据缓冲区大小
	int dataCacheSize = 500000;

	//删除时候清理
	//需要清理
	SCharList *dataCache = NULL;
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
	

private:

	void ProbeInputBuffer();
	bool OpenDemuxThread();
	void Demux();
	//线程函数
	void run();
	//清理数据
	void Clear();
	//关闭Session
	void Close();

	//需要清理
	AVIOContext* avio = NULL;
	//需要清理
	AVFormatContext* afc = NULL;

	int videoStreamIndex = -1;
	int audioStreamIndex = -1;
	//QMutex yuvMux;
	
	//需要清理
	Decode *decode = NULL;

	int width = 0;
	int height = 0;

private:
	//ReadPacket线程是否运行标志位
	bool isInReadPacketThread = false;

	bool isProbeBuffer = false;

	//是否处于解封装、解码的过程（运行过程）
	bool isRuning = false;

	//需要释放
	char* url = NULL;

	bool isUseReadBuff = false;

};
