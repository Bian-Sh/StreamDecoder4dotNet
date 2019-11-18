#include "StreamDecoder.h"
#include "Session.h"
#include <windows.h>
#include <iostream>
using namespace std;
void _stdcall TimerProcess(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	StreamDecoder::Get()->FixedUpdate();
}

//初始化StreamDecoder 设置日志回调函数
void StreamDecoder::StreamDecoderInitialize(PLog logfunc)
{
	mux.lock();
	if (!Log) Log = logfunc;
	mux.unlock();
	SetTimer(NULL, 1, 40, (TIMERPROC)TimerProcess);
}

//注销StreamDecoder 预留函数
void StreamDecoder::StreamDecoderDeInitialize()
{
	mux.lock();
	Log = NULL;
	mux.unlock();
}


//获取版本号
char* StreamDecoder::GetStreamDecoderVersion()
{
	return "stream decoder version 1.0";
}

//创建一个Session
void* StreamDecoder::CreateSession()
{
	PushLog2Net(Info, "Create Session Success");
	//创建一个缓冲1M的Session
	return new Session(1000000);
}

//删除一个Session
void StreamDecoder::DeleteSession(void* session)
{
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

//尝试打开解封装线程
bool StreamDecoder::OpenDemuxThread(void* session, int waitDemuxTime)
{
	Session* s = (Session*)session;
	if (s == NULL)
	{
		PushLog2Net(Error, "OpenDemuxThread exception, session is null");
		return false;
	}
	return s->OpenDemuxThread(waitDemuxTime);
}

//开始解码
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

//停止解码
void StreamDecoder::StopDecode(void* session)
{
	Session* s = (Session*)session;
	if (s == NULL)
	{
		PushLog2Net(Error, "CloseSession exception, session is null");
		return;
	}
	s->Close();
}

//获取 数据流缓冲区 可用空间（字节）
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

//向 数据流缓冲区 追加数据
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
//把消息追加到队列，通过主线程发送
void StreamDecoder::PushLog2Net(LogLevel level, char* log)
{
	LogPacket* packet = new LogPacket(level, log);
	mux.lock();
	logpackets.push_back(packet);
	mux.unlock();
}
//主线程更新 物理时间
void StreamDecoder::FixedUpdate()
{
	mux.lock();
	int size = logpackets.size();
	for (int i = 0; i < size; i++)
	{
		Log2Net(logpackets.front());
		logpackets.pop_front();
	}
	mux.unlock();
}
//调用回调函数（主线程同步）
void StreamDecoder::Log2Net(LogPacket* logpacket)
{
	if (Log && logpacket && logpacket->_log)
	{
		Log(logpacket->_level, logpacket->_log);
	}
	cout << logpacket->_log << endl;
	logpacket->Drop();
	delete logpacket;
	logpacket = NULL;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void StreamDecoderInitialize(PLog logfunc)
{
	StreamDecoder::Get()->StreamDecoderInitialize(logfunc);
}

void StreamDecoderDeInitialize()
{
	StreamDecoder::Get()->StreamDecoderDeInitialize();
}

char* GetStreamDecoderVersion()
{
	return StreamDecoder::Get()->GetStreamDecoderVersion();
}

void* CreateSession()
{
	return StreamDecoder::Get()->CreateSession();
}

void DeleteSession(void* session)
{
	StreamDecoder::Get()->DeleteSession(session);
}


bool OpenDemuxThread(void* session, int waitDemuxTime)
{
	return StreamDecoder::Get()->OpenDemuxThread(session, waitDemuxTime);
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
