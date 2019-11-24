#pragma once

#include <mutex>
class Demux;
class Decode;
class SCharList;
struct AVIOContext;
struct AVFormatContext;
struct AVFrame;

class Session
{

public:
	Session(int playerID, int cacheSize = 1000000);
	~Session();

	void TryStreamDemux(char* url);

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
	

	//设置选项 
	void SetOption(int optionType, int value);
public:

	std::mutex dataCacheMux;

	//退出信号，true 退出线程
	bool isExit = false;

	std::mutex mux;
	



	//是否处于Demux状态 重要的作用在ReadPacket的回调函数中
	bool isDemuxing = false;
	//bool isStreamInfoFinding = false;

	//解封装前的时间戳ns
	int64_t startTime = 0;

	bool waitQuitSignal = false;

private:


	Demux* demux = NULL;

	int dataCacheSize = 1000000;

	//解封装等待时间
	int demuxTimeout = 2000;
	int pushFrameInterval = 0;
	bool alwaysWaitBitStream = false;
	int waitBitStreamTimeout = 1000;

#pragma region 老
	void OpenDemuxThread(char* url);

	//线程函数
	void run();
	//清理数据
	void Clear();
	//关闭Session
	void Close();


	int videoStreamIndex = -1;
	int audioStreamIndex = -1;
	//QMutex yuvMux;

	//需要清理
	Decode *decode = NULL;

	int width = 0;
	int height = 0;
	//ReadPacket线程是否运行标志位
	bool isInReadPacketThread = false;
	bool isInterruptRead = false;

	//bool isProbeBuffer = false;

	//是否处于解封装、解码的过程（运行过程）
	bool isRuning = false;

	//需要释放
	char* url = NULL;

	int playerID;

	bool isLandscape = false;
#pragma endregion 老

	

};
