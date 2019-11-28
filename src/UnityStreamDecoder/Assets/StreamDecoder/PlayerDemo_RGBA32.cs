using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using SStreamDecoder;
using UnityEngine.UI;
using System.Threading;
using System.IO;


public class PlayerDemo_RGBA32 : MonoBehaviour
{
 
    public int readBuffSize = 1024;
    public string localPath = "F:/HTTPServer/Faded.mp4";
    public string netUrl = "rtmp://192.168.30.135/live/test";
    private bool isExit = false;
    
    public RawImage rimg;
    private Material mat;
    private int width = 0;
    private int height = 0;
    private Texture2D tex;

   
    private StreamPlayer player;
    [Space]
    [Header("StreamPlayer Parameters")]
    [SerializeField]
    private int bitStreamCacheSize = 1000000;
    [SerializeField]
    private int demuxTimeout = 2000;
    [SerializeField]
    private int pushFrameInterval = 20;
    [SerializeField]
    private int waitBitStreamTimeout = 1000;
    [SerializeField]
    private bool alwaysWaitBitStream = true;
    [SerializeField]
    private bool autoDecode = true;
    [SerializeField]
    private int decodeThreadCount = 4;


    // Use this for initialization
    void Start()
    {

#if UNITY_EDITOR
        StreamDecoder.dllPath = Application.streamingAssetsPath + "/../../../../bin/";
#else
        StreamDecoder.dllPath = Application.streamingAssetsPath + "/../../../../../bin/";
#endif

        //加载动态库
        if (!StreamDecoder.LoadLibrary())
        {
            return;
        }

        mat = rimg.material;

    }

    private void OnDestroy()
    {
        isExit = true;
        DeleteSession();
        //释放动态库
        StreamDecoder.FreeLibrary();
    }

  
    public void CreateSession()
    {
        if (player != null) return;
  
        player = StreamPlayer.CreateSession();
        player.SetOption(OptionType.DataCacheSize, bitStreamCacheSize);
        player.SetOption(OptionType.DemuxTimeout, demuxTimeout);
        player.SetOption(OptionType.PushFrameInterval, pushFrameInterval);
        player.SetOption(OptionType.AlwaysWaitBitStream, alwaysWaitBitStream ? 1 : 0);
        player.SetOption(OptionType.WaitBitStreamTimeout, waitBitStreamTimeout);
        player.SetOption(OptionType.AutoDecode, autoDecode ? 1 : 0);
        player.SetOption(OptionType.DecodeThreadCount, decodeThreadCount);
        player.SetOption(OptionType.UseCPUConvertYUV, 1);
        player.SetPlayerCb(OnEvent,OnDrawFrame);

    }
    private void OnEvent(SessionEventType type)
    {
        if(type == SessionEventType.DemuxSuccess)
        {
            Debug.Log("Demux Success");
        }
    }

    public void OnDrawFrame(Frame frame)
    {
        if(frame.rgb == System.IntPtr.Zero)
        {
            Debug.Log("Please set UseCPUConvertYUV");
            return;
        }
        if (mat == null)
        {
            Debug.LogWarning("mat is null");
            return;
        }
        if (width != frame.width || height != frame.height)
        {
            width = frame.width;
            height = frame.height;
            tex = new Texture2D(width, height, TextureFormat.RGBA32, false);
        }
        tex.LoadRawTextureData(frame.rgb, width * height * 4);
        tex.Apply();
        mat.SetTexture("_RawImg", tex);
      
    }
    public void DeleteSession()
    {
        if (player == null) return;

        StreamPlayer.DeleteSession(ref player);
    }

    public void TryBitStreamDemux()
    {
        if (player == null) return;
        player.TryBitStreamDemux();
    }
    public void TryNetStreamDemux()
    {
        if (player == null) return;
        player.TryNetStreamDemux(netUrl);
    }

  
    public void BeginDecode()
    {
        if (player == null) return;
        player.BeginDecode();
    }
    public void StopDecode()
    {
        if (player == null) return;
        player.StopDecode();
    }
    public void GetCacheFreeSize()
    {
        if (player == null) return;
        Debug.Log(player.GetCacheFreeSize());
    }

#region Send Data
    private bool isSending = false;
    public void StartSendData()
    {
        if (isSending) return;
        isSending = true;
        new Thread(run).Start();
    }
    public void EndSendData()
    {
        if (!isSending) return;
        isSending = false;
    }

    private void run()
    {
        Debug.Log("Begin send Data");

        if (!File.Exists(localPath))
        {
            Debug.Log(localPath + " not exists");
            return;
        }
        FileStream file = new FileStream(localPath, FileMode.Open);
        byte[] readBuff = new byte[readBuffSize];
        int count = 0;
        while (!isExit && isSending)
        {
            int ret = 0;
            try
            {
                ret = file.Read(readBuff, 0, readBuffSize);
            }
            catch (System.Exception ex)
            {
                Debug.LogWarning(ex);
                return;
            }
            if (ret <= 0)
            {
                break;
            }
            count += ret;
            //处理数据
            while (!isExit && isSending)
            {
                if(player == null)
                {
                    break;
                }
                if (player.PushStream2Cache(readBuff, ret)) break;
              
                Thread.Sleep(1);
                continue;
            }
            Thread.Sleep(1);
        }
        file.Dispose();
        file.Close();
        Debug.Log("Stop send data");
    }
#endregion
}
