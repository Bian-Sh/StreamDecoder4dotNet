using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using StreamDecoderManager;
using UnityEngine.UI;
using System.Threading;
using System.IO;

public class GameManager : MonoBehaviour
{

    private DecodeSession session;
    public Text tipText;
    public string path = "F:/HTTPServer/Faded.mp4";
    private bool isRunthread = false;
    private bool isExit = false;
    private FileStream file;
    public RawImage rimg;
    private Material mat;
    private int width = 0;
    private int height = 0;
    private Texture2D ytex, utex, vtex;
    // Use this for initialization
    void Start()
    {
        tipText.text = Application.streamingAssetsPath;
#if UNITY_EDITOR
        StreamDecoder.dllPath = Application.streamingAssetsPath + "/../../../../bin/";
#else
        StreamDecoder.dllPath = Application.streamingAssetsPath + "/../../../../../bin/";
#endif

        //加载动态库
        if (!StreamDecoder.LoadLibrary()) return;
        StreamDecoder.InitializeStreamDecoder();
        StreamDecoder.SetStreamDecoderPushFrameInterval(20);
        StreamDecoder.logEvent += StreamDecoderLog;
        StreamDecoder.drawEvent += OnDrawFrame;

        mat = rimg.material;

        
    }

    // Update is called once per frame
    void Update()
    {

    }
    private void OnDestroy()
    {
        isExit = true;
        //确保关闭
        DeleteSession();
        StreamDecoder.DeInitializeStreamDecoder();
        //释放动态库
        StreamDecoder.FreeLibrary();
    }

    public void CreateSession()
    {
        if (session != null) return;
        session = DecodeSession.CreateSession();

    }
    public void DeleteSession()
    {
        if (session == null) return;
        isExit = true;
        DecodeSession.DeleteSession(ref session);
    }

    public void OpenDemuxThread()
    {
        if (session == null) return;
        Debug.Log(session.OpenDemuxThread(2000));
        if (isRunthread) return;
        isExit = false;
        new Thread(run).Start();
    }
   
    private void run()
    {
        Debug.Log("begin run");
        if(!File.Exists(path))
        {
            Debug.Log(path + " not exists");
            return;
        }
        file = new FileStream(path, FileMode.Open);
        isRunthread = true;
        byte[] readBuff = new byte[1024];
        int count = 0;
        while(!isExit)
        {
            int ret = 0;
            try
            {
                ret = file.Read(readBuff, 0, 1024);
            }
            catch (System.Exception ex)
            {
                Debug.LogWarning(ex);
                return;
            }
            if(ret <= 0)
            {
                break;
            }
            count += ret;
            //处理数据
            while(!isExit)
            {
                if(session.PushStream2Cache(readBuff, ret)) break;
               
                Thread.Sleep(1);
                continue;
            }
            Thread.Sleep(1);
        }
        file.Dispose();
        file.Close();
        isRunthread = false;
        Debug.Log("stop run");
    }
    public void BeginDecode()
    {
        if (session == null) return;
        session.BeginDecode();
    }
    public void StopDecode()
    {
        if (session == null) return;
        session.StopDecode();
        isExit = true;
    }
    public void GetCacheFreeSize()
    {
        if (session == null) return;
        Debug.Log(session.GetCacheFreeSize());
    }

    private void StreamDecoderLog(int level, string log)
    {
        if (level == 0)
        {
            tipText.text = string.Format("<color=#ffffff>{0}</color>", log);
            Debug.Log(log);
        }
        else if (level == 1)
        {
            tipText.text = string.Format("<color=#ffff00>{0}</color>", log);
            Debug.LogWarning(log);
        }
        else
        {
            tipText.text = string.Format("<color=#ff0000>{0}</color>", log);
            Debug.LogError(log);
        }
    }
    public void OnDrawFrame(DotNetFrame frame)
    {

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
    }
}
