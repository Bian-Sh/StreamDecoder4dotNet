using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;
using StreamDecoderManager;

public class DecodeSession {
    
    private IntPtr session;
    //创建一个DecodeSession并返回
    public static DecodeSession CreateSession()
    {
        return new DecodeSession(Native.Invoke<IntPtr, StreamDecoder.CreateSession>(StreamDecoder.streamDecoder_dll));
    }
    //关闭并删除一个Session
    public static void DeleteSession(ref DecodeSession session)
    {
        if (session == null)
        {
            Debug.LogError("session is null");
            return;
        }
        Native.Invoke<StreamDecoder.DeleteSession>(StreamDecoder.streamDecoder_dll, session.session);
        session = null;
    }

    private DecodeSession(IntPtr session)
    {
        if(session == IntPtr.Zero)
        {
            Debug.Log("session is null");
            return;
        }
        this.session = session;
    }

    //获取版本信息
    public string GetStreamDecoderVersion()
    {
        IntPtr versionPtr = Native.Invoke<IntPtr, StreamDecoder.GetStreamDecoderVersion>(StreamDecoder.streamDecoder_dll);
        return Marshal.PtrToStringAnsi(versionPtr);
    }
    //尝试打开解封装线程
    public bool TryDemux(int waitDemuxTime)
    {
        if (session == null)
        {
            Debug.LogError("session is null");
            return false;
        }
        return Native.Invoke<bool, StreamDecoder.TryDemux>(StreamDecoder.streamDecoder_dll, session, waitDemuxTime);
    }
    public bool TryNetStreamDemux(string url)
    {
        if (session == null)
        {
            Debug.LogError("session is null");
            return false;
        }
        return Native.Invoke<bool, StreamDecoder.TryNetStreamDemux>(StreamDecoder.streamDecoder_dll, session, url);
    }

    //开始解码
    public void BeginDecode()
    {
        if (session == null)
        {
            Debug.LogError("session is null");
            return;
        }
        Native.Invoke<StreamDecoder.BeginDecode>(StreamDecoder.streamDecoder_dll, session);
    }

    //停止解码
    public void StopDecode()
    {
        if (session == null)
        {
            Debug.LogError("session is null");
            return;
        }
        Native.Invoke<StreamDecoder.StopDecode>(StreamDecoder.streamDecoder_dll, session);
    }

    public int GetCacheFreeSize()
    {
        if (session == null) return -1;
        return Native.Invoke<int, StreamDecoder.GetCacheFreeSize>(StreamDecoder.streamDecoder_dll, session);
    }

    public bool PushStream2Cache(byte[] data, int len)
    {
        if (session == null) return false;
        return Native.Invoke<bool, StreamDecoder.PushStream2Cache>(StreamDecoder.streamDecoder_dll, session, data, len);
    }
}
