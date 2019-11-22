#pragma once

#include <mutex>
class Decode;
class SCharList;
struct AVIOContext;
struct AVFormatContext;
struct AVFrame;

class Session
{

public:
	Session(int playerID, int dataCacheSize = 1000000);
	~Session();

	//���ֽ�������
	bool TryBitStreamDemux();
	//��rtmp
	bool TryNetStreamDemux(char* url);
	//��ʼ����
	void BeginDecode();
	//ֹͣ����
	void StopDecode();
	
	//��ȡ �ֽ������� ����ռ��С
	int GetCacheFreeSize();
	//���������
	bool PushStream2Cache(char* data, int len);

	//������ɺ�Decode����
	void OnDecodeOnFrame(AVFrame *frame);
	

	//����ѡ�� 
	void SetOption(int optionType, int value);
public:

	//���ݻ�������С Ĭ��1M
	int dataCacheSize = 1000000;

	//ɾ��ʱ������
	//��Ҫ����
	SCharList *dataCache = NULL;
	std::mutex dataCacheMux;

	//�˳��źţ�true �˳��߳�
	bool isExit = false;

	std::mutex mux;
	

	//���װ�ȴ�ʱ��
	int demuxTimeout = 2000;
	int pushFrameInterval = 0;
	bool alwaysWaitBitStream = false;
	int waitBitStreamTimeout = 1000;

	//�Ƿ���Demux״̬ ��Ҫ��������ReadPacket�Ļص�������
	bool isDemuxing = false;

	//���װǰ��ʱ���ns
	int64_t startTime = 0;

	bool waitQuitSignal = false;

private:

	void ProbeInputBuffer();
	bool OpenDemuxThread();
	void Demux();
	//�̺߳���
	void run();
	//��������
	void Clear();
	//�ر�Session
	void Close();

	//��Ҫ����
	AVIOContext* avio = NULL;
	//��Ҫ����
	AVFormatContext* afc = NULL;

	int videoStreamIndex = -1;
	int audioStreamIndex = -1;
	//QMutex yuvMux;
	
	//��Ҫ����
	Decode *decode = NULL;

	int width = 0;
	int height = 0;

private:
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

};
