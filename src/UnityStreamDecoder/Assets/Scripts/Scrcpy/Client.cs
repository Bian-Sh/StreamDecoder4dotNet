using System;
using System.Collections.Generic;
using UnityEngine;
using System.Net;
using System.Net.Sockets;
using System.Threading;

public class Client {

    private Socket socket;
    private bool isVideo;
    private bool isExit = false;

    private const int MAX_LENGTH = 10240;
    private byte[] readBuff = new byte[MAX_LENGTH];
    private readonly Mutex mutex = new Mutex();
    public Client(Socket socket, bool isVideo)
    {
        this.socket = socket;
        this.isVideo = isVideo;
    }
    public void Start()
    {
        try
        {
            socket.BeginReceive(readBuff, 0, MAX_LENGTH, SocketFlags.None, ReceiveCb, null);
        }
        catch(Exception ex)
        {
            Debug.LogError(ex);
        }
    }

    private void ReceiveCb(IAsyncResult ar)
    {
        if (isExit) return;

        try
        {
            mutex.WaitOne();
            int len = socket.EndReceive(ar);
            if(len <= 0)
            {
                mutex.ReleaseMutex();
                Debug.LogWarning("客户端关闭");
                return;
            }
            //是接受Video数据的Socket
            if(isVideo)
            {
                byte[] buff = new byte[len];
                Buffer.BlockCopy(readBuff, 0, buff, 0, len);
                Scrcpy.Instance.PushToDataCache(buff);
            }
            else
            {
                Debug.Log("control data");
            }
            socket.BeginReceive(readBuff, 0, MAX_LENGTH, SocketFlags.None, ReceiveCb, null);
            mutex.ReleaseMutex();
        }
        catch(Exception ex)
        {
            mutex.ReleaseMutex();
            Debug.LogError(ex);
        }
    }

    public void Close()
    {
        isExit = true;
        mutex.WaitOne();
        if (socket.Connected)
        {
            socket.Shutdown(SocketShutdown.Both);
            socket.Close();
        }

        mutex.ReleaseMutex();
    }

    public void SendControlCmd(byte[] cmd)
    {
        if (socket.Connected)
        {
            socket.Send(cmd);
        }
    }
}
