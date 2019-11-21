#pragma once
#include <mutex>
#include <list>
#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC 
#endif

#ifdef _WIN32 //包含win32 和win64
#define HEAD EXTERNC __declspec(dllexport)
#else
#define HEAD EXTERNC
#endif
//#pragma pack(1)
enum LogLevel
{
	Info,
	Warning,
	Error
};
struct LogPacket;
struct Frame;
struct DotNetFrame;
typedef void(*PLog)(int level, char* log);
typedef void(*PDrawFrame)(DotNetFrame* frame);

class StreamDecoder
{
	
public:
	
	inline static StreamDecoder* Get()
	{
	
		static StreamDecoder sp;
		return &sp;
	}

	//初始化StreamDecoder 设置日志回调函数
	void StreamDecoderInitialize(PLog logfunc, PDrawFrame drawfunc);

	void SetPushFrameInterval(int wait);

	//注销StreamDecoder 预留函数
	void StreamDecoderDeInitialize();


	//获取版本号
	char* GetStreamDecoderVersion();
	//创建一个Session
	void* CreateSession();
	//删除一个Session
	void DeleteSession(void* session);

	//尝试打开解封装线程
	bool TryBitStreamDemux(void* session, int waitDemuxTime);

	bool TryNetStreamDemux(void* session, char* url);

	//开始解码
	void BeginDecode(void* session);
	//停止解码
	void StopDecode(void* session);

	//获取 数据流缓冲区 可用空间（字节）
	int GetCacheFreeSize(void* session);
	//向 数据流缓冲区 追加数据
	bool PushStream2Cache(void* session, char* data, int len);

	//把消息追加到队列，通过主线程发送
	void PushLog2Net(LogLevel level, char* log);
	//
	void PushFrame2Net(Frame* frame);
	//主线程更新 物理时间
	void FixedUpdate();
private:
	StreamDecoder();
	//调用回调函数（主线程同步）
	void Log2Net(LogPacket* logpacket);

	void DrawFrame2dotNet(Frame* frame);

private:
	std::mutex logMux;
	PLog Log = NULL;

	std::mutex frameMux;
	PDrawFrame DrawFrame = NULL;

	std::list<LogPacket*> logpackets;
	std::list<Frame*> framepackets;

	int waitPushFrameTime = 0;
};

HEAD void _cdecl StreamDecoderInitialize(PLog logfunc, PDrawFrame drawfunc);

HEAD void _cdecl StreamDecoderDeInitialize();

HEAD char* _cdecl GetStreamDecoderVersion();

HEAD void* _cdecl CreateSession();

HEAD void _cdecl DeleteSession(void* session);

HEAD bool _cdecl TryBitStreamDemux(void* session, int waitDemuxTime);

HEAD bool _cdecl TryNetStreamDemux(void* session, char* url);

HEAD void _cdecl BeginDecode(void* session);

HEAD void _cdecl StopDecode(void* session);

HEAD int _cdecl GetCacheFreeSize(void* session);

HEAD bool _cdecl PushStream2Cache(void* session, char* data, int len);

HEAD void _cdecl SetPushFrameInterval(int wait);