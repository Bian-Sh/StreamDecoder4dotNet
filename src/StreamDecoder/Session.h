#pragma once

#include <mutex>
class Demux;
class Decode;
struct AVFrame;
struct AVPacket;
struct SessionConfig;
class Session
{

public:
	Session(int playerID, int cacheSize = 1000000);
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
	

	//����ѡ�� 
	void SetOption(int optionType, int value);
public:

	SessionConfig* config;

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


};
