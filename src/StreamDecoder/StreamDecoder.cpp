#include "StreamDecoder.h"
#include "Session.h"
#include <windows.h>
#include <iostream>
#include "Packet.h"
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

}
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
using namespace std;
void _stdcall TimerProcess(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	StreamDecoder::Get()->FixedUpdate();
}

StreamDecoder::StreamDecoder()
{
	//��ʼ��FFmpeg
	av_register_all();
	avcodec_register_all();
	avformat_network_init();
}
//��ʼ��StreamDecoder ������־�ص�����
void StreamDecoder::StreamDecoderInitialize(PLog logfunc, PDrawFrame drawfunc, PEvent ev)
{
	logMux.lock();
	if (!Log) Log = logfunc;
	logMux.unlock();

	frameMux.lock();
	if (!DrawFrame) DrawFrame = drawfunc;
	frameMux.unlock();

	eventMux.lock();
	if (!Event)Event = ev;
	eventMux.unlock();
	
	SetTimer(NULL, 1, 25, (TIMERPROC)TimerProcess);
}

//ע��StreamDecoder Ԥ������
void StreamDecoder::StreamDecoderDeInitialize()
{
	logMux.lock();
	Log = NULL;
	logMux.unlock();

	frameMux.lock();
	DrawFrame = NULL;
	frameMux.unlock();
}


//��ȡ�汾��
char* StreamDecoder::GetStreamDecoderVersion()
{
	return "stream decoder version 1.0";
}

//����һ��Session
void* StreamDecoder::CreateSession(int playerID, int dataCacheSize)
{
	PushLog2Net(Info, "Create Session Success");
	return new Session(playerID, dataCacheSize);
}

//ɾ��һ��Session
void StreamDecoder::DeleteSession(void* session)
{
	//if(IsBadReadPtr(this, sizeof(Session)))
	Session* s = (Session*)session;
	if (s == NULL)
	{
		PushLog2Net(Error, "DeleteSession exception, session is null");
		return;
	}
	delete s;
	s = NULL;
	PushLog2Net(Info, "Delete Session Success");
}

//���Դ򿪽��װ�߳�
void StreamDecoder::TryBitStreamDemux(void* session)
{
	Session* s = (Session*)session;
	if (s == NULL)
	{
		PushLog2Net(Error, "TryBitStreamDemux exception, session is null");
		return;
	}
	s->TryStreamDemux(NULL);
}

void StreamDecoder::TryNetStreamDemux(void* session, char* url)
{
	Session* s = (Session*)session;
	if (s == NULL)
	{
		PushLog2Net(Error, "TryNetStreamDemux exception, session is null");
		return;
	}
	s->TryStreamDemux(url);
}

//��ʼ����
void StreamDecoder::BeginDecode(void* session)
{
	Session* s = (Session*)session;
	if (s == NULL)
	{
		PushLog2Net(Error, "BeginDecode exception, session is null");
		return;
	}
	s->BeginDecode();
}

//ֹͣ����
void StreamDecoder::StopDecode(void* session)
{
	Session* s = (Session*)session;
	if (s == NULL)
	{
		PushLog2Net(Error, "CloseSession exception, session is null");
		return;
	}
	s->StopDecode();
}

//��ȡ ������������ ���ÿռ䣨�ֽڣ�
int StreamDecoder::GetCacheFreeSize(void* session)
{
	Session* s = (Session*)session;
	if (s == NULL)
	{
		PushLog2Net(Error, "GetCacheFreeSize exception, session is null");
		return false;
	}
	return s->GetCacheFreeSize();
}

//�� ������������ ׷������
bool StreamDecoder::PushStream2Cache(void* session, char* data, int len)
{

	Session* s = (Session*)session;
	if (s == NULL)
	{
		PushLog2Net(Error, "PushStream2Cache exception, session is null");
		return false;
	}
	return s->PushStream2Cache(data, len);
}

void StreamDecoder::SetOption(void* session, int optionType, int value)
{
	Session* s = (Session*)session;
	if (s == NULL)
	{
		PushLog2Net(Error, "SetOption exception, session is null");
		return;
	}
	s->SetOption(optionType, value);
}

//����Ϣ׷�ӵ����У�ͨ�����̷߳���
void StreamDecoder::PushLog2Net(LogLevel level, char* log)
{
	LogPacket* packet = new LogPacket(level, log);
	logMux.lock();
	logpackets.push_back(packet);
	logMux.unlock();
}

void StreamDecoder::PushFrame2Net(Frame* frame)
{
	frameMux.lock();
	framepackets.push_back(frame);
	frameMux.unlock();
}

void StreamDecoder::PushEvent2Net(int playerID, int eventType)
{
	DEvent * ev = new DEvent(playerID, eventType);
	eventMux.lock();
	eventpackets.push_back(ev);
	eventMux.unlock();
}

//���̸߳��� ����ʱ��
void StreamDecoder::FixedUpdate()
{
	//callTime++;
	logMux.lock();
	int size = logpackets.size();
	for (int i = 0; i < size; i++)
	{
		Log2Net(logpackets.front());
		logpackets.pop_front();
	}
	logMux.unlock();

	frameMux.lock();
	size = framepackets.size();
	for (int i = 0; i < size; i++)
	{
		DrawFrame2dotNet(framepackets.front());
		framepackets.pop_front();
	}
	frameMux.unlock();

	eventMux.lock();
	size = eventpackets.size();
	for (int i = 0; i < size; i++)
	{
		Event2Net(eventpackets.front());
		eventpackets.pop_front();
	}
	eventMux.unlock();
}



//���ûص����������߳�ͬ����
void StreamDecoder::Log2Net(LogPacket* logpacket)
{
	if (Log && logpacket && logpacket->_log)
	{
		Log(logpacket->_level, logpacket->_log);
	}
	cout << logpacket->_log << endl;
	delete logpacket;
	logpacket = NULL;
}
//���ûص����������߳�ͬ����
void StreamDecoder::DrawFrame2dotNet(Frame* frame)
{
	if (DrawFrame)
	{
		DotNetFrame* dotNetFrame = new DotNetFrame();
		dotNetFrame->playerID = frame->playerID;
		dotNetFrame->width = frame->width;
		dotNetFrame->height = frame->height;
		dotNetFrame->frame_y = frame->frame_y;
		dotNetFrame->frame_u = frame->frame_u;
		dotNetFrame->frame_v = frame->frame_v;
		//��������C#
		DrawFrame(dotNetFrame);
		//Log2Net(new LogPacket(Info, "call ok"));
		delete dotNetFrame;
		dotNetFrame = NULL;
	}
	delete frame;
	frame = NULL;
}

void StreamDecoder::Event2Net(DEvent* ev)
{
	if (Event) Event(ev->playerID, ev->eventType);
	delete ev;
	ev = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void StreamDecoderInitialize(PLog logfunc, PDrawFrame drawfunc, PEvent ev)
{
	StreamDecoder::Get()->StreamDecoderInitialize(logfunc, drawfunc, ev);
}

void StreamDecoderDeInitialize()
{
	StreamDecoder::Get()->StreamDecoderDeInitialize();
}

char* GetStreamDecoderVersion()
{
	return StreamDecoder::Get()->GetStreamDecoderVersion();
}

void* CreateSession(int playerID, int dataCacheSize)
{
	return StreamDecoder::Get()->CreateSession(playerID, dataCacheSize);
}

void DeleteSession(void* session)
{
	StreamDecoder::Get()->DeleteSession(session);
}


void TryBitStreamDemux(void* session)
{
	StreamDecoder::Get()->TryBitStreamDemux(session);
}

void TryNetStreamDemux(void* session, char* url)
{
	StreamDecoder::Get()->TryNetStreamDemux(session, url);
}

void BeginDecode(void* session)
{
	StreamDecoder::Get()->BeginDecode(session);
}

void StopDecode(void* session)
{
	StreamDecoder::Get()->StopDecode(session);
}

int GetCacheFreeSize(void* session)
{
	return StreamDecoder::Get()->GetCacheFreeSize(session);
}

bool PushStream2Cache(void* session, char* data, int len)
{
	return StreamDecoder::Get()->PushStream2Cache(session, data, len);
}

void SetOption(void* session, int optionType, int value)
{
	StreamDecoder::Get()->SetOption(session, optionType, value);
}

