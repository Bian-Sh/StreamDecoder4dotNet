#pragma once
#include <mutex>
#include <list>
class Demux;
class Decode;
struct AVFrame;
struct AVPacket;
struct SessionConfig;

struct Frame;
typedef void(*PEvent)(int playerID, int eventType);
typedef void(*PDrawFrame)(Frame* frame);

class Session
{

public:
	Session(int playerID, PEvent pE, PDrawFrame pDF);
	~Session();

	void TryStreamDemux(char* url);

	//��ʼ����
	void BeginDecode();
	//ֹͣ����
	void StopDecode();
	
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


public:

	SessionConfig* config = NULL;

	//�˳��źţ�true �˳��߳�
	bool quitSignal = false;

	std::mutex mux;

private:
	void OpenDemuxThread(char* url);

	//��������
	void Clear();
	//�ر�Session
	void Close();
private:
	Demux* demux = NULL;

	//��Ҫ����
	Decode *vdecode = NULL;

	int width = 0;
	int height = 0;
	bool isLandscape = false;


	std::mutex funcMux;

	PEvent DotNetSessionEvent = NULL;
	PDrawFrame DotNetDrawFrame = NULL;

	std::list<Frame*> framePackets;
	std::list<int> eventPackets;

};
