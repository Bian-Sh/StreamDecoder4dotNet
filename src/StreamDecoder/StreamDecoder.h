#pragma once
#include <mutex>
#include <list>
#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC 
#endif

#ifdef _WIN32 //����win32 ��win64
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

	//��ʼ��StreamDecoder ������־�ص�����
	void StreamDecoderInitialize(PLog logfunc);
	//ע��StreamDecoder Ԥ������
	void StreamDecoderDeInitialize();


	//��ȡ�汾��
	char* GetStreamDecoderVersion();
	//����һ��Session
	void* CreateSession();
	//ɾ��һ��Session
	void DeleteSession(void* session);

	//���Դ򿪽��װ�߳�
	bool OpenDemuxThread(void* session, int waitDemuxTime);
	//��ʼ����
	void BeginDecode(void* session);
	//ֹͣ����
	void StopDecode(void* session);

	//��ȡ ������������ ���ÿռ䣨�ֽڣ�
	int GetCacheFreeSize(void* session);
	//�� ������������ ׷������
	bool PushStream2Cache(void* session, char* data, int len);

	//����Ϣ׷�ӵ����У�ͨ�����̷߳���
	void PushLog2Net(LogLevel level, char* log);
	//���̸߳��� ����ʱ��
	void FixedUpdate();
private:
	StreamDecoder() {}
	//���ûص����������߳�ͬ����
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