#pragma once
#include <mutex>
#include <list>
class Demux;
class Decode;
struct AVFrame;
struct AVPacket;
struct SessionConfig;

struct Frame;
typedef void(*PEvent)(void* opaque, int eventType);
typedef void(*PDrawFrame)(void* opaque, Frame* frame);

#define VERIFY_VALUE 0x1122334455667788
class Session
{

public:
	Session();
	~Session();

	void TryStreamDemux(char* url);

	//开始解码
	void BeginDecode();
	//停止解码
	void EndDecode();
	
	//获取 字节流缓冲 空余空间大小
	int GetCacheFreeSize();
	//添加数据流
	bool PushStream2Cache(char* data, int len);

	void DemuxSuccess(int width, int height);

	void OnReadOneAVPacket(AVPacket* packet, bool isAudio);
	//解码完成后Decode调用
	void OnDecodeOneAVFrame(AVFrame *frame, bool isAudio);
	
	void Update(bool call_cb = true);

	//设置选项 
	void SetOption(int optionType, int value);

	void SetEventCallBack(PEvent pEvent, PDrawFrame pDrawFrame, void* opaque);

	inline bool IsVaild()
	{
		if (verifyValue == VERIFY_VALUE) return true;
		else return false;
	}

public:

	SessionConfig* config = NULL;

	//退出信号，true 退出线程
	bool quitSignal = false;

	std::mutex mux;

private:
	void OpenDemuxThread();

	//清理数据
	void Clear();
	//关闭Session
	void Close();
private:

	char* url = NULL;

	Demux* demux = NULL;

	//需要清理
	Decode *vdecode = NULL;

	int width = 0;
	int height = 0;
	bool isLandscape = false;


	std::mutex funcMux;

	void* opaque = NULL;

	PEvent DotNetSessionEvent = NULL;
	PDrawFrame DotNetDrawFrame = NULL;

	std::list<Frame*> framePackets;
	std::list<int> eventPackets;

	long long verifyValue;
};
