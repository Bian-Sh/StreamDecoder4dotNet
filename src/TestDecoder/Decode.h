#pragma once

#include <QThread>
#include <QMutex>
#include <list>
class Decode : public QThread
{
	Q_OBJECT

public:
	Decode(class Session *session);
	~Decode();

	//���۴�����ͷ� AVCodecParameters
	bool Open(struct AVCodecParameters *para);

	void Push(struct AVPacket *pkt);

	//��������������ģ����رգ�ָ����ΪNULL
	void Close();

protected:
	void run();

private:
	QMutex mux;

	//�����������ģ���Ҫ�ͷ� avcodec_free_context
	struct AVCodecContext* codec = NULL;

	//��Ҫ����
	std::list<AVPacket*> packets;

	bool isExit = false;

	Session* session = NULL;
};
