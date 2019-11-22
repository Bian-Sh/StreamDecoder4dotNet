using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text.RegularExpressions;
using UnityEngine;
using UnityEngine.UI;

public class QScrcpy : MonoBehaviour
{

    private List<byte> dataCache = new List<byte>();
    public GameObject devicesObject;
    public Transform devicesListParent;
    public InputField deviceSerialInput;
    public InputField deviceIPInput;
    public Toggle isWifi;
    public int reverserPort = 5555;
    private static QScrcpy instance;
    private bool isExit = false;
    private AdbController adbController;

    private Socket tcpServer;
    public static QScrcpy Instance
    {
        get { return instance; }
    }

    private event System.Action UpdateEvent;

    private void Awake()
    {
        adbController = new AdbController();
        instance = this;
    }
    private FileStream fs;
    // Use this for initialization
    void Start()
    {
        fs = new FileStream("D:/device.h264", FileMode.CreateNew);
        ADBStart();
    }
    private void OnDestroy()
    {
        //if(client != null)
        //{
        //    if(client.Connected)
        //    {
        //        client.Shutdown(SocketShutdown.Both);
        //        client.Close();
        //        Debug.Log("关闭client");
        //    }
        //}
        fs.Close();
        ADBKill();

        isExit = true;
    }
    bool isFirst = true;

   
    // Update is called once per frame
    void Update()
    {
        if (UpdateEvent != null)
            UpdateEvent();

        if (dataCache.Count <= 0) return;
        if (isFirst)
        {
            lock(dataCache)
            {
                if (dataCache.Count < 68) return;
                isFirst = false;
                byte[] ba = dataCache.ToArray();
                string devicename = System.Text.Encoding.Default.GetString(ba, 0, 64);
                Debug.Log("device name:" + devicename);
                Debug.Log("width:" + BitConverter.ToUInt16(new byte[2] { ba[65], ba[64] }, 0));
                Debug.Log("height:" + BitConverter.ToInt16(new byte[2] { ba[67], ba[66] }, 0));
                dataCache.RemoveRange(0, 64);
            }
           
        }

        lock(dataCache)
        {
            byte[] arr = dataCache.ToArray();
            fs.Write(arr, 0, arr.Length);
            dataCache.Clear();
        }
        
    }
    public void SetEvent(bool isAdd, System.Action ac)
    {
        if (isAdd) UpdateEvent += ac;
        else UpdateEvent -= ac;
    }
    public void TryScrcpy()
    {
        PushQScrcpy();
    }
    public void ClearDevices()
    {
        RemoveQScrcpy();
        CloseReverseProxy();
    }

    public void GetDevices()
    {
        adbController.GetDevices((list) =>
        {
            ClearDeviceList();
            for (int i = 0; i < list.Count; i++)
            {
                GameObject obj = Instantiate(devicesObject, devicesListParent);
                obj.GetComponentInChildren<Text>().text = list[i];
            }
        });
    }
    public void ADBStart()
    {
        adbController.AdbStartServer();
    }
    public void ADBKill()
    {
        adbController.AdbKillServer();
    }

    public void ClearDeviceList()
    {
        Transform[] allTrans = devicesListParent.GetComponentsInChildren<Transform>();
        for (int i = 0; i < allTrans.Length; i++)
        {
            if (allTrans[i] != devicesListParent.transform)
                Destroy(allTrans[i].gameObject);
        }
    }
    public void SetSelectDevice(string serial)
    {
        deviceSerialInput.text = serial;
        ClearDeviceList();
    }

    public void GetDeviceIP()
    {
        adbController.GetDeviceIP(deviceSerialInput.text, isWifi.isOn, (ip) => { deviceIPInput.text = ip; });
    }

    private void PushQScrcpy()
    {
        string local = Application.streamingAssetsPath + "/scrcpy-server.jar";
        adbController.PushQScrcpy(deviceSerialInput.text, local, (isSuccess) =>
        {
            if (isSuccess)
            {
                Debug.Log("push success");
                OpenReverseProxy();
            }
        });
    }

    private void OpenReverseProxy()
    {

        adbController.OpenReverseProxy(deviceSerialInput.text, reverserPort, (isSuccess) =>
        {
            if (isSuccess)
            {
                Debug.Log("open reverse success localabstract:scrcpy tcp:" + reverserPort);
                //打开tcp服务器
                tcpServer = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                try
                {
                    tcpServer.Bind(new IPEndPoint(IPAddress.Parse("127.0.0.1"), reverserPort));
                    tcpServer.Listen(0);
                    tcpServer.BeginAccept(AcceptCb, null);
                    Debug.Log("服务器启动成功");
                    StartQScrcpyServer();
                }
                catch (Exception ex)
                {
                    Debug.Log(ex.ToString());
                }

                //打开QScrcpy
                //StartQScrcpyServer();
            }
            else
            {
                adbController.RemoveQScrcpy(deviceSerialInput.text);
            }

        });
    }

    private byte[] readBuff = new byte[1024];
    private Socket client;
    private void AcceptCb(IAsyncResult ar)
    {
        if (isExit)
        {
            Debug.Log("AcceptCb return");
            return;
        }
        try
        {
            client = tcpServer.EndAccept(ar);

            client.BeginReceive(readBuff, 0, 1024, SocketFlags.None, ReceiveCb, null);
        }
        catch (Exception ex)
        {
            Debug.LogWarning(ex);
        }
    }
    private void ReceiveCb(IAsyncResult ar)
    {
        if (isExit)
        {
            Debug.Log("ReceiveCb return");
            return;
        }
        try
        {
            int len = client.EndReceive(ar);
            if (len <= 0)
            {
                client.Shutdown(SocketShutdown.Both);
                client.Close();
                return;
            }
            //数据处理
            byte[] tmpByte = new byte[len];
            Buffer.BlockCopy(readBuff, 0, tmpByte, 0, len);
            lock(dataCache)
            {
                dataCache.AddRange(tmpByte);
            }
            client.BeginReceive(readBuff, 0, 1024, SocketFlags.None, ReceiveCb, null);
        }
        catch (Exception ex)
        {
            Debug.LogWarning(ex);
        }
    }
    private void RemoveQScrcpy()
    {
        adbController.RemoveQScrcpy(deviceSerialInput.text);
    }

    private void CloseReverseProxy()
    {
        adbController.CloseReverseProxy(deviceSerialInput.text);
    }

    private void StartQScrcpyServer()
    {
        adbController.StartQScrcpyServer(deviceSerialInput.text, 0, 20000000);
    }

}
