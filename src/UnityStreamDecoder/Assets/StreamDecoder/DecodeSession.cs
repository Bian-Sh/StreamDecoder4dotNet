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
   
    //关闭一个Session
    public void CloseSession()
    {
        if (session == null)
        {
            Debug.LogError("session is null");
            return;
        }
        Native.Invoke<StreamDecoder.CloseSession>(StreamDecoder.streamDecoder_dll, session);
    }

    public bool OpenDemuxThread(int waitDemuxTime)
    {
        if (session == null)
        {
            Debug.LogError("session is null");
            return false;
        }
        return Native.Invoke<bool, StreamDecoder.OpenDemuxThread>(StreamDecoder.streamDecoder_dll, session, waitDemuxTime);
    }

}
