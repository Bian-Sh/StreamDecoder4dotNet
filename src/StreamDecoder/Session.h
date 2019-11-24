#pragma once

#include <mutex>
class Demux;
class Decode;
struct AVFrame;
struct AVPacket;

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

	void OnReadOneAVPacket(AVPacket* packet, bool isAudio);
	//������ɺ�Decode����
	void OnDecodeOneAVFrame(AVFrame *frame);
	

	//����ѡ�� 
	void SetOption(int optionType, int value);
public:

	std::mutex dataCacheMux;

	//�˳��źţ�true �˳��߳�
	//bool isExit = false;

	std::mutex mux;


	bool waitQuitSignal = false;

private:
	void OpenDemuxThread(char* url);

	//��������
	void Clear();
	//�ر�Session
	void Close();
private:
	Demux* demux = NULL;

	int dataCacheSize = 1000000;

	//���װ�ȴ�ʱ��
	int demuxTimeout = 2000;
	int pushFrameInterval = 0;
	bool alwaysWaitBitStream = false;
	int waitBitStreamTimeout = 1000;

#pragma region ��
	

	//�̺߳���
	void run();
	
	//QMutex yuvMux;

	//��Ҫ����
	Decode *decode = NULL;

	int width = 0;
	int height = 0;
	//ReadPacket�߳��Ƿ����б�־λ
	bool isInReadPacketThread = false;
	bool isInterruptRead = false;

	//bool isProbeBuffer = false;

	//�Ƿ��ڽ��װ������Ĺ��̣����й��̣�
	bool isRuning = false;

	//��Ҫ�ͷ�
	char* url = NULL;

	int playerID;

	bool isLandscape = false;
#pragma endregion ��

	

};
