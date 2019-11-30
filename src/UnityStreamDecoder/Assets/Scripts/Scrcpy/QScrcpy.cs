using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using UnityEngine;
using UnityEngine.UI;
using SStreamDecoder;
using System.IO;

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
    //private Socket udpClient;
    //private IPEndPoint remoteIEP;

    private StreamPlayer player;
    public static QScrcpy Instance
    {
        get { return instance; }
    }

    public event Action UpdateEvent;

    public bool writeToLocal = false;
    public string fileName = "D:/device.h264";

    
    public bool useQtScrcpy = false;
    private void Awake()
    {

        //int a = 0x11223344;
        //byte[] bab = System.BitConverter.GetBytes(a);

        adbController = new AdbController();
        instance = this;

        //remoteIEP = new IPEndPoint(IPAddress.Parse("192.168.30.135"), 5555);
        //udpClient = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
        //SendTo("==========Start Debug Unity==========");
    }
    //public void SendTo(string msg)
    //{
    //    udpClient.SendTo(System.Text.Encoding.ASCII.GetBytes(msg), remoteIEP);
    //}
    private FileStream fsWrite;

    private bool isFirst = true;
    public RawImage rimg;
    private Material mat;
    private int width = 0;
    private int height = 0;
    public int Width { get { return width; } }
    public int Height { get { return height; } }
    private Texture2D ytex, utex, vtex;
    // Use this for initialization
    void Start()
    {
#if UNITY_EDITOR
        StreamDecoder.dllPath = Application.streamingAssetsPath + "/../../../../bin/";
#else
        StreamDecoder.dllPath = Application.streamingAssetsPath + "/../../../../../bin/";
#endif
        adbController.adbPath = StreamDecoder.dllPath + "adb.exe";

        if (writeToLocal)
        {
            if (File.Exists(fileName)) fsWrite = new FileStream(fileName, FileMode.Truncate);
            else fsWrite = new FileStream(fileName, FileMode.Create);
        }

        StreamDecoder.LoadLibrary();
        player = StreamPlayer.CreateSession();
        player.SetOption(OptionType.DataCacheSize, 2000000);
        player.SetOption(OptionType.DemuxTimeout, 2000);
        //player.SetOption(OptionType.PushFrameInterval, 0);
        player.SetOption(OptionType.AlwaysWaitBitStream, 1);
        //player.SetOption(OptionType.WaitBitStreamTimeout, waitBitStreamTimeout);
        player.SetOption(OptionType.AutoDecode, 1);
        player.SetOption(OptionType.DecodeThreadCount, 0);
        //player.SetOption(OptionType.UseCPUConvertYUV, 0);
        //player.SetOption(OptionType.ConvertPixelFormat, (int)PixelFormat.RGBA);
        //player.SetOption(OptionType.AsyncUpdate, 0);
        player.SetPlayerCb(null, OnFrame);
        mat = rimg.material;
        ADBStart();


    }
    // Update is called once per frame
    void Update()
    {

        if (UpdateEvent != null)
            UpdateEvent();

        if (dataCache.Count <= 0) return;
        if (isFirst)
        {
            lock (dataCache)
            {
                //64个字节为设备名称，2个字节为宽，2个字节为高
                if (dataCache.Count < 68) return;
                isFirst = false;
                byte[] ba = dataCache.ToArray();
                string devicename = System.Text.Encoding.Default.GetString(ba, 0, 64);
                Debug.Log("device name:" + devicename);
                Debug.Log("width:" + BitConverter.ToUInt16(new byte[2] { ba[65], ba[64] }, 0));
                Debug.Log("height:" + BitConverter.ToInt16(new byte[2] { ba[67], ba[66] }, 0));
                dataCache.RemoveRange(0, 64);
                player.TryBitStreamDemux();
                //做清理标记
                ClearDevices();
            }

        }

        lock (dataCache)
        {

            byte[] arr = dataCache.ToArray();
            int size = Mathf.Min(player.GetCacheFreeSize(), arr.Length);
            if (player.PushStream2Cache(arr, size))
            {
                if (writeToLocal)
                {
                    fsWrite.Write(arr, 0, size);
                }
                dataCache.RemoveRange(0, size);
            }
        }
    }
    private void OnDestroy()
    {
        if (writeToLocal)
        {
            fsWrite.Close();
        }

        StreamPlayer.DeleteSession(ref player);
        StreamDecoder.FreeLibrary();
        //ADBKill();
        if (qtScrcpyServer != null)
        {
            qtScrcpyServer.Kill();
        }
        isExit = true;
    }


    void OnFrame(Frame frame)
    {
        if (mat == null)
        {
            Debug.LogWarning("mat is null");
            return;
        }
        if (width != frame.width || height != frame.height)
        {
            width = frame.width;
            height = frame.height;
            ytex = new Texture2D(width, height, TextureFormat.R8, false);
            utex = new Texture2D(width / 2, height / 2, TextureFormat.R8, false);
            vtex = new Texture2D(width / 2, height / 2, TextureFormat.R8, false);
            //width 不大于 1200 高度不大于1000

            if (width > height)
            {
                float rate = height / (float)width;
                rimg.rectTransform.sizeDelta = new Vector2(1200, 1200 * rate);
            }
            else
            {
                float rate = width / (float)height;
                rimg.rectTransform.sizeDelta = new Vector2(1000 * rate, 1000);
            }
        }
        ytex.LoadRawTextureData(frame.frame_y, width * height);
        ytex.Apply();
        utex.LoadRawTextureData(frame.frame_u, width * height / 4);
        utex.Apply();
        vtex.LoadRawTextureData(frame.frame_v, width * height / 4);
        vtex.Apply();
        mat.SetTexture("_YTex", ytex);
        mat.SetTexture("_UTex", utex);
        mat.SetTexture("_VTex", vtex);
    }

    //public void SetEvent(bool isAdd, System.Action ac)
    //{
    //    if (isAdd) UpdateEvent += ac;
    //    else UpdateEvent -= ac;
    //}
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

        CloseServer();
        adbController.AdbKillServer();

    }

    public void CloseServer()
    {
        rimg.rectTransform.sizeDelta = new Vector2(0, 0);
        lock (dataCache)
        {
            dataCache.Clear();
            isFirst = true;
            if (player != null)
                player.StopDecode();
        }
        if (client != null && client.Connected)
        {
            lock (client)
            {
                client.Shutdown(SocketShutdown.Both);
                client.Close();
                client = null;
            }
        }

        if (tcpServer != null)
        {
            lock (tcpServer)
            {
                tcpServer.Close();
                tcpServer = null;
            }

        }
        if (qtScrcpyServer != null)
        {
            qtScrcpyServer.Kill();
        }

        width = 0;
        height = 0;
        player.StopDecode();
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
        string local = StreamDecoder.dllPath + (useQtScrcpy ? "scrcpy-server_qt.jar" : "scrcpy-server.jar");
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
                if (tcpServer != null)
                {
                    Debug.LogWarning("当前服务已经启动");
                    return;
                }
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
    public Socket client;
    private void AcceptCb(IAsyncResult ar)
    {
        if (isExit)
        {
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
            lock (dataCache)
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

    private Process qtScrcpyServer;
    private void StartQScrcpyServer()
    {
        qtScrcpyServer = adbController.StartQScrcpyServer(deviceSerialInput.text, 0, 10000000);
    }

}
