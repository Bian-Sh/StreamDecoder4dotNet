using System;
using System.Collections.Generic;
using UnityEngine;
using System.Net;
using System.Net.Sockets;
using System.Threading;

public class NetManager {

    private int port;
    private Socket tcpServer;
    private EndPoint endPoint;
    private bool isExit = false;

    private readonly Mutex mutex = new Mutex();
    private bool isVideoSocket = true;

    private Client videoClient;
    private Client controlClient;

	public bool Start(int port)
    {
        this.port = port;
        endPoint = new IPEndPoint(IPAddress.Loopback, port);
        tcpServer = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
        try
        {
            tcpServer.Bind(endPoint);
            tcpServer.Listen(2);
            tcpServer.BeginAccept(AcceptCb, null);
            Debug.Log("TCP服务器启动成功");
            return true;
        }
        catch(Exception ex)
        {
            Debug.LogError(ex.ToString());
            return false;
        }
        
    }

    private void AcceptCb(IAsyncResult ar)
    {
        if (isExit) return;

        if(isVideoSocket)
        {
            isVideoSocket = false;
           
            try
            {
                videoClient = new Client(tcpServer.EndAccept(ar), true);
                videoClient.Start();
            }
            catch(Exception ex)
            {
                Debug.LogError(ex);
            }
        }
        else
        {
            
            try
            {
                controlClient = new Client(tcpServer.EndAccept(ar), false);
                controlClient.Start();
            }
            catch (Exception ex)
            {
                Debug.LogError(ex);
            }
        }

        tcpServer.BeginAccept(AcceptCb, null);
    }

  
    public void Close()
    {
        isExit = true;

        mutex.WaitOne();
        tcpServer.Close();
        tcpServer = null;

        if(videoClient != null)
        {
            videoClient.Close();
            videoClient = null;
        }
        if(controlClient != null)
        {
            controlClient.Close();
            controlClient = null;
        }

        mutex.ReleaseMutex();
    }

    public void SendControlCmd(byte[] cmd)
    {
        if(controlClient!= null)
        {
            controlClient.SendControlCmd(cmd);
        }
    }
}
