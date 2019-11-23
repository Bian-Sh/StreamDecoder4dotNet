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

	//��ʼ��StreamDecoder ������־�ص�����
	void StreamDecoderInitialize(PLog logfunc, PDrawFrame drawfunc, PEvent ev);

	//ע��StreamDecoder Ԥ������
	void StreamDecoderDeInitialize();


	//��ȡ�汾��
	char* GetStreamDecoderVersion();
	//����һ��Session
	void* CreateSession(int playerID, int dataCacheSize);
	//ɾ��һ��Session
	void DeleteSession(void* session);

	//���Դ򿪽��װ�߳�
	bool TryBitStreamDemux(void* session);

	bool TryNetStreamDemux(void* session, char* url);

	//��ʼ����
	void BeginDecode(void* session);
	//ֹͣ����
	void StopDecode(void* session);

	//��ȡ ������������ ���ÿռ䣨�ֽڣ�
	int GetCacheFreeSize(void* session);
	//�� ������������ ׷������
	bool PushStream2Cache(void* session, char* data, int len);

	//���ò���
	void SetOption(void* session, int optionType, int value);

	//����Ϣ׷�ӵ����У�ͨ�����̷߳���
	void PushLog2Net(LogLevel level, char* log);
	//
	void PushFrame2Net(Frame* frame);

	void PushEvent2Net(int playerID, int eventType);

	//���̸߳��� ����ʱ��
	void FixedUpdate();

private:
	StreamDecoder();
	//���ûص����������߳�ͬ����
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