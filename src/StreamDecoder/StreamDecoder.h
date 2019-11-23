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
struct DEvent;
typedef void(*PLog)(int level, char* log);
typedef void(*PDrawFrame)(DotNetFrame* frame);
typedef void(*PEvent)(int playerID, int eventType);
class StreamDecoder
{
	
public:
	
	inline static StreamDecoder* Get()
	{
	
		static StreamDecoder sp;
		return &sp;
	}

	//初始化StreamDecoder 设置日志回调函数
	void StreamDecoderInitialize(PLog logfunc, PDrawFrame drawfunc, PEvent ev);

	//注销StreamDecoder 预留函数
	void StreamDecoderDeInitialize();


	//获取版本号
	char* GetStreamDecoderVersion();
	//创建一个Session
	void* CreateSession(int playerID, int dataCacheSize);
	//删除一个Session
	void DeleteSession(void* session);

	//尝试打开解封装线程
	bool TryBitStreamDemux(void* session);

	bool TryNetStreamDemux(void* session, char* url);

	//开始解码
	void BeginDecode(void* session);
	//停止解码
	void StopDecode(void* session);

	//获取 数据流缓冲区 可用空间（字节）
	int GetCacheFreeSize(void* session);
	//向 数据流缓冲区 追加数据
	bool PushStream2Cache(void* session, char* data, int len);

	//设置参数
	void SetOption(void* session, int optionType, int value);

	//把消息追加到队列，通过主线程发送
	void PushLog2Net(LogLevel level, char* log);
	//
	void PushFrame2Net(Frame* frame);

	void PushEvent2Net(int playerID, int eventType);

	//主线程更新 物理时间
	void FixedUpdate();

private:
	StreamDecoder();
	//调用回调函数（主线程同步）
	void Log2Net(LogPacket* logpacket);

	void DrawFrame2dotNet(Frame* frame);

	void Event2Net(DEvent* ev);
private:
	std::mutex logMux;
	PLog Log = NULL;

	std::mutex frameMux;
	PDrawFrame DrawFrame = NULL;

	std::mutex eventMux;
	PEvent Event = NULL;

	std::list<LogPacket*> logpackets;
	std::list<Frame*> framepackets;
	std::list<DEvent*> eventpackets;
	//int waitPushFrameTime = 0;
};

//Global
HEAD void _cdecl StreamDecoderInitialize(PLog logfunc, PDrawFrame drawfunc, PEvent ev);
//Global
HEAD void _cdecl StreamDecoderDeInitialize();
//Global
HEAD char* _cdecl GetStreamDecoderVersion();

HEAD void* _cdecl CreateSession(int playerID, int dataCacheSize);

HEAD void _cdecl DeleteSession(void* session);

HEAD bool _cdecl TryBitStreamDemux(void* session);

HEAD bool _cdecl TryNetStreamDemux(void* session, char* url);

HEAD void _cdecl BeginDecode(void* session);

HEAD void _cdecl StopDecode(void* session);

HEAD int _cdecl GetCacheFreeSize(void* session);

HEAD bool _cdecl PushStream2Cache(void* session, char* data, int len);

HEAD void _cdecl SetOption(void* session, int optionType, int value);