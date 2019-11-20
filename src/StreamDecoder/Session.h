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
	bool TryDemux(int waitDemuxTime);
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

	static char* av_strerror2(int errnum);
	//������ɺ�Decode����
	void OnDecodeOnFrame(AVFrame *frame);
	
	//�ر�Session
	void Close();

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

	//�Ƿ��ڽ��װ������Ĺ��̣����й��̣�
	bool isRuning = false;

	//��Ҫ�ͷ�
	static char* logbuf;

	//��Ҫ�ͷ�
	char* url = NULL;

};
