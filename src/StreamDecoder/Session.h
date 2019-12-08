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

	//��ʼ����
	void BeginDecode();
	//ֹͣ����
	void EndDecode();
	
	//��ȡ �ֽ������� ����ռ��С
	int GetCacheFreeSize();
	//���������
	bool PushStream2Cache(char* data, int len);

	void DemuxSuccess(int width, int height);

	void OnReadOneAVPacket(AVPacket* packet, bool isAudio);
	//������ɺ�Decode����
	void OnDecodeOneAVFrame(AVFrame *frame, bool isAudio);
	
	void Update(bool call_cb = true);

	//����ѡ�� 
	void SetOption(int optionType, int value);

	void SetEventCallBack(PEvent pEvent, PDrawFrame pDrawFrame, void* opaque);

	inline bool IsVaild()
	{
		if (verifyValue == VERIFY_VALUE) return true;
		else return false;
	}

public:

	SessionConfig* config = NULL;

	//�˳��źţ�true �˳��߳�
	bool quitSignal = false;

	std::mutex mux;

private:
	void OpenDemuxThread();

	//��������
	void Clear();
	//�ر�Session
	void Close();
private:

	char* url = NULL;

	Demux* demux = NULL;

	//��Ҫ����
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
