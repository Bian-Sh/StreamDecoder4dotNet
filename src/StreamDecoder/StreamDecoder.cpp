#include "StreamDecoder.h"
#include "Session.h"
#include "Tools.h"
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
#pragma comment(lib, "yuv.lib")
using namespace std;
void _stdcall TimerProcess(HWND hwnd, UINT uMsg, UINT_PTR timerPtr, DWORD dwTime)
{
	StreamDecoder::Get()->FixedUpdate();
}
//void CALLBACK TimerProc(HWND hWnd, UINT nMsg, UINT nTimerid, DWORD dwTime)
//{
//	cout << nTimerid << endl;
//}

StreamDecoder::StreamDecoder()
{
	//初始化FFmpeg
	av_register_all();
	avcodec_register_all();
	avformat_network_init();
}


StreamDecoder::~StreamDecoder()
{
	StreamDecoderDeInitialize();
	
}

//初始化StreamDecoder 设置日志回调函数
void StreamDecoder::StreamDecoderInitialize(PLog logfunc)
{

	if (!Log) Log = logfunc;
	
	/*if (!DotNetSessionEvent) DotNetSessionEvent = pE;

	if (!DotNetDrawFrame) DotNetDrawFrame = pDF;*/

	timerPtr = SetTimer(NULL, 0, 15, (TIMERPROC)TimerProcess);
	startTimeStamp = Tools::Get()->GetTimestamp();
}

//注销StreamDecoder
void StreamDecoder::StreamDecoderDeInitialize()
{
	logMux.lock();
	Log = NULL;
	logMux.unlock();

	KillTimer(NULL, timerPtr);
	cout << "平均刷新率" << GetUpdateRate() << "次/s" << endl;
}


//获取版本号
char* StreamDecoder::GetStreamDecoderVersion()
{
	return "stream decoder version 1.1";
}

//创建一个Session
void* StreamDecoder::CreateSession(int playerID)
{
	Session* session = new Session(playerID);
	sessionList.push_back(session);
	PushLog2Net(Info, "Create Session Success");
	return session;
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

	if (std::count(sessionList.begin(), sessionList.end(), session) == 1)
	{
		vector<Session*>::iterator iter = find(sessionList.begin(), sessionList.end(), session);
		sessionList.erase(iter);
	}
	else
	{
		cout << "严重错误，不存在当前值 session" << endl;
		PushLog2Net(Error, "Serious Error! Remove session from vector<Session*>");
		return;
	}
	
	/*if (!s->IsVaild())
	{
		PushLog2Net(Error, "Serious Error! Invaild Sessiion on DeleteSession");
		return;
	}*/

	delete s;
	s = NULL;
	PushLog2Net(Info, "Delete Session Success");
}

//尝试打开解封装线程
void StreamDecoder::TryBitStreamDemux(void* session)
{
	Session* s = (Session*)session;
	if (s == NULL)
	{
		PushLog2Net(Error, "TryBitStreamDemux exception, session is null");
		return;
	}
	if (!s->IsVaild())
	{
		PushLog2Net(Error, "Serious Error! Invaild Sessiion on TryBitStreamDemux");
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
	if (!s->IsVaild())
	{
		PushLog2Net(Error, "Serious Error! Invaild Sessiion on TryNetStreamDemux");
		return;
	}
	s->TryStreamDemux(url);
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
	if (!s->IsVaild())
	{
		PushLog2Net(Error, "Serious Error! Invaild Sessiion on BeginDecode");
		return;
	}
	s->BeginDecode();
}

//停止解码
void StreamDecoder::EndDecode(void* session)
{
	Session* s = (Session*)session;
	if (s == NULL)
	{
		PushLog2Net(Error, "CloseSession exception, session is null");
		return;
	}
	if (!s->IsVaild())
	{
		PushLog2Net(Error, "Serious Error! Invaild Sessiion on EndDecode");
		return;
	}
	s->EndDecode();
}

//获取 数据流缓冲区 可用空间（字节）
int StreamDecoder::GetCacheFreeSize(void* session)
{
	Session* s = (Session*)session;
	if (s == NULL)
	{
		PushLog2Net(Error, "GetCacheFreeSize exception, session is null");
		return -2;
	}

	if (!s->IsVaild())
	{
		PushLog2Net(Error, "Serious Error! Invaild Sessiion on GetCacheFreeSize");
		return -2;
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
	if (!s->IsVaild())
	{
		PushLog2Net(Error, "Serious Error! Invaild Sessiion on PushStream2Cache");
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
	if (!s->IsVaild())
	{
		PushLog2Net(Error, "Serious Error! Invaild Sessiion on SetOption");
		return;
	}
	s->SetOption(optionType, value);
}


void StreamDecoder::SetEventCallBack(void* session, void(*PEvent)(int playerID, int eventType), void(*PDrawFrame)(Frame* frame))
{
	Session* s = (Session*)session;
	if (s == NULL)
	{
		PushLog2Net(Error, "SetEventCallBack exception, session is null");
		return;
	}
	if (!s->IsVaild())
	{
		PushLog2Net(Error, "Serious Error! Invaild Sessiion on SetEventCallBack");
		return;
	}

	s->SetEventCallBack(PEvent, PDrawFrame);
}

//把消息追加到队列，通过主线程发送
void StreamDecoder::PushLog2Net(LogLevel level, char* log)
{
	LogPacket* packet = new LogPacket(level, log);
	logMux.lock();
	logpackets.push_back(packet);
	logMux.unlock();
}


//主线程更新 物理时间
void StreamDecoder::FixedUpdate()
{

	int sessionCount = sessionList.size();
	for (int i = 0; i < sessionCount; i ++)
	{
		sessionList[i]->Update();
	}

	timerCounter++;
	logMux.lock();
	int size = logpackets.size();
	for (int i = 0; i < size; i++)
	{
		Log2Net(logpackets.front());
		logpackets.pop_front();
	}
	logMux.unlock();

}


//获取平均更新率
int StreamDecoder::GetUpdateRate()
{
	long long aliveTime = Tools::Get()->GetTimestamp() - startTimeStamp;
	return (int)((float)timerCounter * 1000 / aliveTime);
}

//调用回调函数（主线程同步）
void StreamDecoder::Log2Net(LogPacket* logpacket)
{
	if (Log && logpacket && logpacket->_log)
	{
		Log(logpacket->_level, logpacket->_log);
	}
	else
	{
		cout << logpacket->_log << endl;
	}
	
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

void* CreateSession(int playerID)
{
	return StreamDecoder::Get()->CreateSession(playerID);
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

void EndDecode(void* session)
{
	StreamDecoder::Get()->EndDecode(session);
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

void SetEventCallBack(void* session, void(*PEvent)(int playerID, int eventType), void(*PDrawFrame)(Frame* frame))
{
	StreamDecoder::Get()->SetEventCallBack(session, PEvent, PDrawFrame);
}

