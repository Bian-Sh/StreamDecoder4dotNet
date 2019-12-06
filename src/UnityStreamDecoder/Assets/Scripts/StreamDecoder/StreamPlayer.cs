using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;
using SStreamDecoder;

public class StreamPlayer {

    /// <summary>
    /// Session指针
    /// </summary>
    private IntPtr session = IntPtr.Zero;
    private int playerID;

    
    /// <summary>
    /// 创建一个StreamDecoder
    /// </summary>
    /// <param name="playerID">唯一ID</param>
    /// <param name="onReceiveEventCb">事件回调</param>
    /// <param name="onReceiveOneFrameCb">绘制回调</param>
    /// <returns></returns>
    public static StreamPlayer CreateSession()
    {
        int id = StreamDecoder.GetNewID();
        if(!StreamDecoder.IsInit)
        {
            StreamDecoder.InitStreamDecoder();
        }
        IntPtr s = Native.Invoke<IntPtr, StreamDecoder.CreateSession>(StreamDecoder.streamDecoder_dll, id);
        return new StreamPlayer(s, id);
    }
    /// <summary>
    /// 关闭并删除一个Session
    /// </summary>
    /// <param name="session"></param>
    public static void DeleteSession(ref StreamPlayer session)
    {
        session.DestructorStreamPlayer();
        Native.Invoke<StreamDecoder.DeleteSession>(StreamDecoder.streamDecoder_dll, session.session);
        session.session = IntPtr.Zero;
        session = null;
    }


    /// <summary>
    /// 构造StreamPlayer
    /// </summary>
    /// <param name="session"></param>
    private StreamPlayer(IntPtr session, int playerID)
    {
        if(session == IntPtr.Zero)
        {
            Debug.LogError("Session is null");
            return;
        }
        this.playerID = playerID;
        this.session = session;

        StreamDecoder.onFrameEvent += OnFrame;
        StreamDecoder.onSessionEvent += OnSessionEvent;

    }
    public void DestructorStreamPlayer()
    {
        StreamDecoder.onFrameEvent -= OnFrame;
        StreamDecoder.onSessionEvent -= OnSessionEvent;
    }
    //尝试打开解封装线程
    public void TryBitStreamDemux()
    {
        if (session == IntPtr.Zero)
        {
            Debug.LogError("session is null");
            return;
        }
        Native.Invoke<StreamDecoder.TryBitStreamDemux>(StreamDecoder.streamDecoder_dll, session);
    }
    public void TryNetStreamDemux(string url)
    {
        if (session == IntPtr.Zero)
        {
            Debug.LogError("session is null");
            return;
        }
       Native.Invoke<StreamDecoder.TryNetStreamDemux>(StreamDecoder.streamDecoder_dll, session, url);
    }

    //开始解码
    public void BeginDecode()
    {
        if (session == IntPtr.Zero)
        {
            Debug.LogError("session is null");
            return;
        }
        Native.Invoke<StreamDecoder.BeginDecode>(StreamDecoder.streamDecoder_dll, session);
    }

    //停止解码
    public void StopDecode()
    {
        if (session == IntPtr.Zero)
        {
            Debug.LogError("session is null");
            return;
        }
        Native.Invoke<StreamDecoder.EndDecode>(StreamDecoder.streamDecoder_dll, session);
    }

    public int GetCacheFreeSize()
    {
        if (session == IntPtr.Zero) return -1;
        return Native.Invoke<int, StreamDecoder.GetCacheFreeSize>(StreamDecoder.streamDecoder_dll, session);
    }

    public bool PushStream2Cache(byte[] data, int len)
    {
        if (session == IntPtr.Zero) return false; 
        return Native.Invoke<bool, StreamDecoder.PushStream2Cache>(StreamDecoder.streamDecoder_dll, session, data, len);
    }

    public void SetOption(OptionType type, int value)
    {
        if (session == IntPtr.Zero) return;
        Native.Invoke<StreamDecoder.SetOption>(StreamDecoder.streamDecoder_dll, session, (int)type, value);
    }

    private void OnSessionEvent(int playerID, SessionEventType type)
    {
        if (playerID != this.playerID) return;
        if (onSessionEvent != null) onSessionEvent(type);
    }
    private void OnFrame(Frame frame)
    {
        if (playerID != frame.playerID) return;
        if (onFrameEvent != null) onFrameEvent(frame);
    }

    private Action<SessionEventType> onSessionEvent;
    private Action<Frame> onFrameEvent;
    public void SetPlayerCb(Action<SessionEventType> onSessionEvent, Action<Frame> onFrameEvent)
    {
        this.onSessionEvent = onSessionEvent;
        this.onFrameEvent = onFrameEvent;
    }

    public void SetEventCallBack(Action<int ,int> onSessionEvent, Action<Frame> onFrameEvent, object opaque)
    {
        if (session == IntPtr.Zero) return;
        StreamDecoder._SetEventCallBack(session, opaque);
    }
}
