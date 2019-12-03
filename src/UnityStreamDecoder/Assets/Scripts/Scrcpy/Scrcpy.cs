using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using UnityEngine;
using UnityEngine.UI;
using SStreamDecoder;
using System.IO;
using System.Threading;

public class Scrcpy : MonoBehaviour
{
 
    private List<byte> dataCache = new List<byte>();


    private List<byte[]> cmdList = new List<byte[]>();

    /// <summary>
    /// 设备列表UI元素prefab
    /// </summary>
    public GameObject devicesObject;
    /// <summary>
    /// 列表布局容器
    /// </summary>
    public Transform devicesListParent;
    /// <summary>
    /// 序列号输入框, IP输入框
    /// </summary>
    public InputField deviceSerialInput;
    public InputField deviceIPInput;
    /// <summary>
    /// 是否是无线连接
    /// </summary>
    public Toggle isWifi;
   
    private static Scrcpy instance;
    public static Scrcpy Instance { get { return instance; } }
    /// <summary>
    /// 退出标记
    /// </summary>
    private bool isExit = false;
    /// <summary>
    /// ADB命令集成
    /// </summary>
    private AdbController adbController;

    /// <summary>
    /// 画面显示RawImage
    /// </summary>
    public RawImage rimg;
    /// <summary>
    /// RawImage材质
    /// </summary>
    private Material mat;

    /// <summary>
    /// Update更新事件
    /// </summary>
    public event Action UpdateEvent;



    private int width = 0;
    private int height = 0;
    public int Width { get { return width; } }
    public int Height { get { return height; } }
    private Texture2D ytex, utex, vtex;


    public Image loadImg;

    /// <summary>
    /// Scrcpy进程
    /// </summary>
    private Process qtScrcpyServer;

    /// <summary>
    /// 网络管理
    /// </summary>
    public NetManager netManager;

    /// <summary>
    /// 字节流播放器
    /// </summary>
    public StreamPlayer player;

    private bool isNeedDecodeHead = true;
    private bool isInTryScrcpy = false;

    /// <summary>
    /// 反向代理端口
    /// </summary>
    public int reverserPort = 5555;
    public int maxSize = 0;
    public int bitRate = 10000000;
    private void Awake()
    {
        instance = this;
        adbController = new AdbController();
        
    }
   
    // Use this for initialization
    void Start()
    {

        rimg.transform.parent.gameObject.SetActive(false);
        mat = rimg.material;
#if UNITY_EDITOR
        StreamDecoder.dllPath = Application.streamingAssetsPath + "/../../../../bin/";
#else
        StreamDecoder.dllPath = Application.streamingAssetsPath + "/../../../../../bin/";
#endif
        adbController.adbPath = StreamDecoder.dllPath + "adb.exe";

        //启动ADB
        adbController.AdbStartServer();


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
        
    }
    // Update is called once per frame
    void Update()
    {

        if(Input.GetKeyDown(KeyCode.Escape))
        {
#if UNITY_EDITOR
            UnityEditor.EditorApplication.isPlaying = false;            
#else
            Application.Quit();
#endif
        }

        loadImg.transform.Rotate(-Vector3.forward * Time.deltaTime * 360);

        HandleCmdList();
        HandleDataCacheList();

        if (UpdateEvent != null)
            UpdateEvent();
    }

    public void PushToDataCache(byte[] buff)
    {
        if (isExit) return;
      
        lock(dataCache)
        {
            dataCache.AddRange(buff);
        }
    }
    private void HandleDataCacheList()
    {
        if (dataCache.Count <= 0) return;
        if (isNeedDecodeHead)
        {
            lock (dataCache)
            {
                //64个字节为设备名称，2个字节为宽，2个字节为高
                if (dataCache.Count < 68) return;
                isNeedDecodeHead = false;
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
                dataCache.RemoveRange(0, size);
            }
        }
    }
    private void HandleCmdList()
    {
        if (cmdList.Count <= 0) return;
        lock (cmdList)
        {
            int size = cmdList.Count;
            for (int i = 0; i < size; i++)
            {
                if(netManager != null)
                {
                    netManager.SendControlCmd(cmdList[i]);
                }
            }
        }
        cmdList.Clear();
    }
    public void SendControlCmd(byte[] cmd)
    {

        lock (cmdList)
        {
            cmdList.Add(cmd);
        }
    }

    /// <summary>
    /// 显示画面
    /// </summary>
    /// <param name="frame"></param>
    void OnFrame(Frame frame)
    {
        if (mat == null)
        {
            Debug.LogWarning("mat is null");
            return;
        }
        if(loadImg.gameObject.activeInHierarchy) loadImg.gameObject.SetActive(false);
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
            rimg.transform.parent.gameObject.SetActive(true);
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


   
    /// <summary>
    /// 获取设备列表
    /// </summary>
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

    /// <summary>
    /// 清理显示设备列表
    /// </summary>
    private void ClearDeviceList()
    {
        Transform[] allTrans = devicesListParent.GetComponentsInChildren<Transform>();
        for (int i = 0; i < allTrans.Length; i++)
        {
            if (allTrans[i] != devicesListParent.transform)
                Destroy(allTrans[i].gameObject);
        }
    }
    /// <summary>
    /// 选择设备Serial
    /// </summary>
    /// <param name="serial"></param>
    public void SetSelectDevice(string serial)
    {
        deviceSerialInput.text = serial;
        ClearDeviceList();
    }

    /// <summary>
    /// 获取设备IP
    /// </summary>
    public void GetDeviceIP()
    {
        adbController.GetDeviceIP(deviceSerialInput.text, isWifi.isOn, (ip) => { deviceIPInput.text = ip; });
    }

    /// <summary>
    /// 尝试启动投屏
    /// </summary>
    public void TryScrcpy()
    {
        if (isInTryScrcpy) return;
        loadImg.gameObject.SetActive(true);
        isInTryScrcpy = true;
        PushScrcpy();
    }
    public void ClearDevices()
    {
        RemoveQScrcpy();
        CloseReverseProxy();
    }


    private void PushScrcpy()
    {
        string local = StreamDecoder.dllPath + "scrcpy-server.jar";
        adbController.PushScrcpy(deviceSerialInput.text, local, (isSuccess) =>
        {
            if (isSuccess)
            {
                Debug.Log("push success");
                OpenReverseProxy();
            }
            else
            {
                isInTryScrcpy = false;
                loadImg.gameObject.SetActive(false);
            }
        });
    }

    private void OpenReverseProxy()
    {

        adbController.OpenReverseProxy(deviceSerialInput.text, reverserPort, (isSuccess) =>
        {
            if (isSuccess)
            {
                Debug.Log("open reverse proxy success");
                //创建服务并启动
                netManager = new NetManager();
                if(netManager.Start(reverserPort))
                {
                    //打开QScrcpy
                    StartQScrcpyServer();
                }
                else
                {
                    //创建失败
                    CloseServer();
                    loadImg.gameObject.SetActive(false);
                }
            }
            else
            {
                isInTryScrcpy = false;
                adbController.RemoveScrcpy(deviceSerialInput.text);
                loadImg.gameObject.SetActive(false);
            }

        });
    }

   
    private void RemoveQScrcpy()
    {
        adbController.RemoveScrcpy(deviceSerialInput.text);
    }

    private void CloseReverseProxy()
    {
        adbController.CloseReverseProxy(deviceSerialInput.text);
    }

    /// <summary>
    /// 启动Scrcpy服务
    /// </summary>
    private void StartQScrcpyServer()
    {
        qtScrcpyServer = adbController.StartQScrcpyServer(deviceSerialInput.text, maxSize, bitRate);
    }

    public void CloseAllServer()
    {
        CloseServer();
        //关闭adb
        adbController.AdbKillServer();
    }

    public void CloseServer()
    {
        //关闭页面显示
        if(rimg!=null)
        {
            rimg.transform.parent.gameObject.SetActive(false);
            rimg.rectTransform.sizeDelta = new Vector2(0, 0);
        }
        if(loadImg != null)
        {
            loadImg.gameObject.SetActive(false);
        }
        

        //清理缓冲
        lock (dataCache)
        {
            dataCache.Clear();
            isNeedDecodeHead = true;
        }

        //停止解码
        if (player != null)
            player.StopDecode();

        //关闭Scrcpy进程
        if (qtScrcpyServer != null)
        {
            qtScrcpyServer.Kill();
        }

        //关闭网络模块
        if (netManager != null)
        {
            netManager.Close();
            netManager = null;
        }


        width = 0;
        height = 0;

        isInTryScrcpy = false;
        isNeedDecodeHead = true;
    }


    private void OnDestroy()
    {

        CloseServer();

        if(player != null)
        {
            StreamPlayer.DeleteSession(ref player);
        }
        
        StreamDecoder.FreeLibrary();
      
        isExit = true;
    }
}
