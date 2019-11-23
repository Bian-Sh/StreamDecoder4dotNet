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
    private Action<DotNetFrame> drawCb;
    private Action<EType> onEvent;
    
    /// <summary>
    /// 创建一个StreamDecoder
    /// </summary>
    /// <param name="playerID">唯一ID</param>
    /// <param name="dataCacheSize">player 流缓冲大小</param>
    /// <param name="onReceiveEventCb">事件回调</param>
    /// <param name="onReceiveOneFrameCb">绘制回调</param>
    /// <returns></returns>
    public static StreamPlayer CreateSession(int playerID, int dataCacheSize, Action<EType> onReceiveEventCb, Action<DotNetFrame> onReceiveOneFrameCb)
    {
        if(!StreamDecoder.IsInit)
        {
            StreamDecoder.InitStreamDecoder();
        }
        IntPtr s = Native.Invoke<IntPtr, StreamDecoder.CreateSession>(StreamDecoder.streamDecoder_dll, playerID, dataCacheSize);
        return new StreamPlayer(s, playerID, onReceiveEventCb, onReceiveOneFrameCb);
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
        session.SetOption(OptionType.AlwaysWaitBitStream, 1);
        session = null;
    }
    private void DestructorStreamPlayer()
    {
        StreamDecoder.drawEvent -= ReceiveOneFrame;
        StreamDecoder.decodeEvent -= OnEvent;
    }

    /// <summary>
    /// 构造StreamPlayer
    /// </summary>
    /// <param name="session"></param>
    private StreamPlayer(IntPtr session, int playerID, Action<EType> onReceiveEventCb, Action<DotNetFrame> onReceiveOneFrameCb)
    {
        if(session == IntPtr.Zero)
        {
            Debug.LogError("Session is null");
            return;
        }
        StreamDecoder.drawEvent += ReceiveOneFrame;
        StreamDecoder.decodeEvent += OnEvent;
        this.playerID = playerID;
        this.session = session;
        if (onReceiveOneFrameCb != null) this.drawCb = onReceiveOneFrameCb;
        if (onReceiveEventCb != null) this.onEvent = onReceiveEventCb;
    }

    //尝试打开解封装线程
    public bool TryBitStreamDemux()
    {
        if (session == IntPtr.Zero)
        {
            Debug.LogError("session is null");
            return false;
        }
        return Native.Invoke<bool, StreamDecoder.TryBitStreamDemux>(StreamDecoder.streamDecoder_dll, session);
    }
    public bool TryNetStreamDemux(string url)
    {
        if (session == IntPtr.Zero)
        {
            Debug.LogError("session is null");
            return false;
        }
        return Native.Invoke<bool, StreamDecoder.TryNetStreamDemux>(StreamDecoder.streamDecoder_dll, session, url);
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
        Native.Invoke<StreamDecoder.StopDecode>(StreamDecoder.streamDecoder_dll, session);
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

    private void ReceiveOneFrame(DotNetFrame frame)
    {
        if (frame.playerID != playerID) return;
        if (drawCb != null) drawCb(frame);
    }

    private void OnEvent(int playerID, EType et)
    {
        if (this.playerID != playerID) return;
        if (onEvent != null) onEvent(et);
    }
}
