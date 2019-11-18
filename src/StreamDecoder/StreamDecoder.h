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

enum LogLevel
{
	Info,
	Warning,
	Error
};
struct LogPacket 
{
	LogLevel _level;
	char* _log = 0;
	LogPacket(LogLevel level, char* log)
	{
		_level = level;
		int size = strlen(log);
		_log = new char[size + 1];
		_log[size] = 0;
		memcpy(_log, log, size);
	}
	void Drop()
	{
		if (_log)
		{
			delete _log;
			_log = NULL;
		}
	}
};
typedef void(*PLog)(int level, char* log);
class StreamDecoder
{

public:
	
	inline static StreamDecoder* Get()
	{
		static StreamDecoder sp;
		return &sp;
	}

	//初始化StreamDecoder 设置日志回调函数
	void StreamDecoderInitialize(PLog logfunc);
	//注销StreamDecoder 预留函数
	void StreamDecoderDeInitialize();


	//获取版本号
	char* GetStreamDecoderVersion();
	//创建一个Session
	void* CreateSession();
	//删除一个Session
	void DeleteSession(void* session);

	//尝试打开解封装线程
	bool OpenDemuxThread(void* session, int waitDemuxTime);
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
	//主线程更新 物理时间
	void FixedUpdate();
private:
	StreamDecoder() {}
	//调用回调函数（主线程同步）
	void Log2Net(LogPacket* logpacket);

private:
	std::mutex mux;
	PLog Log = 0;

	std::list<LogPacket*> logpackets;
};

HEAD void _cdecl StreamDecoderInitialize(PLog logfunc);

HEAD void _cdecl StreamDecoderDeInitialize();

HEAD char* _cdecl GetStreamDecoderVersion();

HEAD void* _cdecl CreateSession();

HEAD void _cdecl DeleteSession(void* session);

HEAD bool _cdecl OpenDemuxThread(void* session, int waitDemuxTime);

HEAD void _cdecl BeginDecode(void* session);

HEAD void _cdecl StopDecode(void* session);

HEAD int _cdecl GetCacheFreeSize(void* session);

HEAD bool _cdecl PushStream2Cache(void* session, char* data, int len);