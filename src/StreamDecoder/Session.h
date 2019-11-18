#pragma once

#include <mutex>
class Decode;

//���뼶���sleep
void Sleep(int ms);

class Session
{

public:
	Session(int dataCacheSize);
	~Session();

	bool TryDemux(int waitDemuxTime);
	bool TryNetStreamDemux(char* url);

	void BeginDecode();
	//
	void StopDecode();

	void Close();
	//���������
	int GetCacheFreeSize();
	bool PushStream2Cache(char* data, int len);

	static char* av_strerror2(int errnum);

	void OnDecodeOnFrame(struct AVFrame *frame);
	
	//���ݻ�������С
	int dataCacheSize = 500000;
	//ɾ��ʱ������
	class SCharList *dataCache = NULL;
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
	

//signals:
//	void OnDemuxSuccessSignal(bool isSuccess, char* info);

private:
	void ProbeInputBuffer();
	bool OpenDemuxThread();
	void Demux();
	//�̺߳���
	void run();

	struct AVIOContext* avio = NULL;
	struct AVFormatContext* afc = NULL;

	int videoStreamIndex = -1;
	int audioStreamIndex = -1;
	//QMutex yuvMux;
	
	//��Ҫ����
	Decode *decode = NULL;

	int width = 0;
	int height = 0;

	//��Ҫ����
	/*unsigned char* yuv[3] = {0};
	int linesizeY = 0;*/

	//bool isDecodeing = false;

private:
	//ReadPacket�߳��Ƿ����б�־λ
	bool isInReadPacketThread = false;

	//�Ƿ��ڽ��װ������Ĺ��̣����й��̣�
	bool isRuning = false;

	static char* logbuf;

	char* url = NULL;
};
