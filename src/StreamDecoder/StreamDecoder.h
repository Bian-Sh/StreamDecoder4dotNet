#pragma once
#include <mutex>
#include <list>
#include <vector>
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

	//��ʼ��StreamDecoder ������־�ص�����
	void StreamDecoderInitialize(PLog logfunc);

	//ע��StreamDecoder
	void StreamDecoderDeInitialize();


	//��ȡ�汾��
	char* GetStreamDecoderVersion();
	//����һ��Session
	void* CreateSession(int playerID);

	//ɾ��һ��Session
	void DeleteSession(void* session);

	//���Դ򿪽��װ�߳�
	void TryBitStreamDemux(void* session);

	void TryNetStreamDemux(void* session, char* url);

	//��ʼ����
	void BeginDecode(void* session);
	//ֹͣ����
	void EndDecode(void* session);

	//��ȡ ������������ ���ÿռ䣨�ֽڣ�
	int GetCacheFreeSize(void* session);
	//�� ������������ ׷������
	bool PushStream2Cache(void* session, char* data, int len);

	//���ò���
	void SetOption(void* session, int optionType, int value);

	void SetEventCallBack(void* session, void(*PEvent)(void* opaque, int playerID, int eventType), void(*PDrawFrame)(void* opaque, Frame* frame), void* opaque);

	//����Ϣ׷�ӵ����У�ͨ�����̷߳���
	void PushLog2Net(LogLevel level, char* log);

	//���̸߳��� ����ʱ��
	void FixedUpdate();

	int GetUpdateRate();
private:
	StreamDecoder();

	//���ûص����������߳�ͬ����
	void Log2Net(LogPacket* logpacket);

private:
	std::mutex logMux;
	//��־�ص�
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