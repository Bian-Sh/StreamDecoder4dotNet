#pragma once
#include <mutex>
#include <list>
#include <vector>
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
	Info = 1,
	Warning,
	Error
};
struct LogPacket;
struct Frame;
//struct DotNetFrame;

class Session;
typedef void(*PLog)(int level, char* log);
//typedef void(*PEvent)(int playerID, int eventType);
//typedef void(*PDrawFrame)(Frame* frame);
class StreamDecoder
{
	
public:
	
	inline static StreamDecoder* Get()
	{
		static StreamDecoder sp;
		return &sp;
	}
	~StreamDecoder();

	//初始化StreamDecoder 设置日志回调函数
	void StreamDecoderInitialize(PLog logfunc);

	//注销StreamDecoder
	void StreamDecoderDeInitialize();


	//获取版本号
	char* GetStreamDecoderVersion();
	//创建一个Session
	void* CreateSession(int playerID);

	//删除一个Session
	void DeleteSession(void* session);

	//尝试打开解封装线程
	void TryBitStreamDemux(void* session);

	void TryNetStreamDemux(void* session, char* url);

	//开始解码
	void BeginDecode(void* session);
	//停止解码
	void EndDecode(void* session);

	//获取 数据流缓冲区 可用空间（字节）
	int GetCacheFreeSize(void* session);
	//向 数据流缓冲区 追加数据
	bool PushStream2Cache(void* session, char* data, int len);

	//设置参数
	void SetOption(void* session, int optionType, int value);

	void SetEventCallBack(void* session, void(*PEvent)(void* opaque, int playerID, int eventType), void(*PDrawFrame)(void* opaque, Frame* frame), void* opaque);

	//把消息追加到队列，通过主线程发送
	void PushLog2Net(LogLevel level, char* log);

	//主线程更新 物理时间
	void FixedUpdate();

	int GetUpdateRate();
private:
	StreamDecoder();

	//调用回调函数（主线程同步）
	void Log2Net(LogPacket* logpacket);

private:
	std::mutex logMux;
	//日志回调
	PLog Log = NULL;
	std::list<LogPacket*> logpackets;

	unsigned long long timerPtr = 0;
	long long timerCounter = 1;
	long long startTimeStamp = 0;

	std::vector<Session*> sessionList;

	/*PEvent DotNetSessionEvent = NULL;
	PDrawFrame DotNetDrawFrame = NULL;*/
};

//Global
HEAD void _cdecl StreamDecoderInitialize(PLog logfunc);
//Global
HEAD void _cdecl StreamDecoderDeInitialize();
//Global
HEAD char* _cdecl GetStreamDecoderVersion();

HEAD void* _cdecl CreateSession(int playerID);

HEAD void _cdecl DeleteSession(void* session);

HEAD void _cdecl TryBitStreamDemux(void* session);

HEAD void _cdecl TryNetStreamDemux(void* session, char* url);

HEAD void _cdecl BeginDecode(void* session);

HEAD void _cdecl EndDecode(void* session);

HEAD int _cdecl GetCacheFreeSize(void* session);

HEAD bool _cdecl PushStream2Cache(void* session, char* data, int len);

HEAD void _cdecl SetOption(void* session, int optionType, int value);

HEAD void _cdecl SetEventCallBack(void* session, void(*PEvent)(void* opaque, int playerID, int eventType), void(*PDrawFrame)(void* opaque, Frame* frame), void* opaque);

HEAD void _cdecl TestSetObj(void* obj);