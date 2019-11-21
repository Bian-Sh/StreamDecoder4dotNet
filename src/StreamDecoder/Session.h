#pragma once

#include <mutex>
class Decode;
class SCharList;
struct AVIOContext;
struct AVFormatContext;
struct AVFrame;

//���뼶���sleep
void Sleep(int ms);

class Session
{

public:
	Session(int dataCacheSize);
	~Session();

	//���ֽ�������
	bool TryBitStreamDemux(int waitDemuxTime);
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
	

public:

	//���ݻ�������С
	int dataCacheSize = 500000;

	//ɾ��ʱ������
	//��Ҫ����
	SCharList *dataCache = NULL;
	std::mutex dataCacheMux;

	//�˳��źţ�true �˳��߳�
	bool isExit = false;

	std::mutex mux;
	

	//���װ�ȴ�ʱ��
	int waitDemuxTime = 2000;
	//�Ƿ���Demux״̬ ��Ҫ��������ReadPacket�Ļص�������
	bool isDemuxing = false;

	//���װǰ��ʱ���ns
	int64_t startTime = 0;
	

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

	bool isProbeBuffer = false;

	//�Ƿ��ڽ��װ������Ĺ��̣����й��̣�
	bool isRuning = false;

	//��Ҫ�ͷ�
	char* url = NULL;

	bool isUseReadBuff = false;

};
