using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using SStreamDecoder;
using UnityEngine.UI;
using System.Threading;
using System.IO;


public class PlayerDemo_YUV : MonoBehaviour
{
 
    public int readBuffSize = 1024;
    public string localPath = "F:/HTTPServer/Faded.mp4";
    public string netUrl = "rtmp://192.168.30.135/live/test";
    private bool applicationIsQuit = false;
    
    public RawImage rimg;
    private Material mat;
    private int width = 0;
    private int height = 0;
    private Texture2D ytex, utex, vtex;

   
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
    [SerializeField]
    private bool useCPUConvertYUV = false;

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
        applicationIsQuit = true;
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
        player.SetOption(OptionType.UseCPUConvertYUV, useCPUConvertYUV ? 1 : 0);
        player.SetOption(OptionType.ConvertPixelFormat, (int)PixelFormat.RGBA);
        player.SetOption(OptionType.AsyncUpdate, 0);
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
        if (mat == null)
        {
            Debug.LogWarning("mat is null");
            return;
        }
        float decodeTime = ((float)(frame.edts - frame.bdts) / decodeThreadCount);

        if (width != frame.width || height != frame.height)
        {
            width = frame.width;
            height = frame.height;
            ytex = new Texture2D(width, height, TextureFormat.R8, false);
            utex = new Texture2D(width / 2, height / 2, TextureFormat.R8, false);
            vtex = new Texture2D(width / 2, height / 2, TextureFormat.R8, false);
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

        //Debug.Log(StreamDecoder.GetTimestamp() - frame.edts + decodeTime);

    }
    public void DeleteSession()
    {
        isInSendThread = false;
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
    private bool isInSendThread = false;
    public void StartSendData()
    {
        if (isInSendThread) return;
        new Thread(run).Start();
    }
    public void EndSendData()
    {
        isInSendThread = false;
    }
    private static readonly Mutex mutex = new Mutex();
    private void run()
    {
     
        Debug.Log("Begin send Data");
        isInSendThread = true;
        if (!File.Exists(localPath))
        {
            Debug.Log(localPath + " not exists");
            goto end;
        }
        FileStream file = new FileStream(localPath, FileMode.Open);
        byte[] readBuff = new byte[readBuffSize];
        int count = 0;
        while (!applicationIsQuit && isInSendThread && player != null)
        {
            int ret = 0;
            try
            {
                ret = file.Read(readBuff, 0, readBuffSize);
                if (ret <= 0)
                {
                    throw new System.Exception("读取到结尾");
                }
            }
            catch (System.Exception ex)
            {
                Debug.LogWarning(ex);
                goto end;
            }
           
            count += ret;
            //处理数据
            while (!applicationIsQuit && isInSendThread)
            {

                mutex.WaitOne();
                if(player == null)
                {
                    mutex.ReleaseMutex();
                    goto end;
                }
                if (player.PushStream2Cache(readBuff, ret))
                {
                    mutex.ReleaseMutex();
                    break;
                }
                mutex.ReleaseMutex();
                Thread.Sleep(1);
                continue;
            }
            Thread.Sleep(1);
        }
        file.Dispose();
        file.Close();

end:
        Debug.Log("Stop send data");
        isInSendThread = false;
    }
#endregion
}
