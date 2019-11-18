using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using StreamDecoderManager;
using UnityEngine.UI;
public class GameManager : MonoBehaviour
{


    private DecodeSession session;
    public Text tipText;

    // Use this for initialization
    void Start()
    {
        //加载动态库
        if (!StreamDecoder.LoadLibrary()) return;

        StreamDecoder.InitializeStreamDecoder();
        StreamDecoder.logEvent += StreamDecoderLog;
    }

    // Update is called once per frame
    void Update()
    {

    }
    private void OnDestroy()
    {
        //确保关闭
        DeleteSession();
        //StreamDecoder.DeInitializeStreamDecoder();
        //释放动态库
        StreamDecoder.FreeLibrary();
    }

    public void CreateSession()
    {
        if (session == null)
        {
            session = DecodeSession.CreateSession();
        }
    }
    public void OpenDemuxThread()
    {
        if (session != null)
        {
            Debug.Log(session.OpenDemuxThread(2000));

        }
    }
    public void DeleteSession()
    {
        if (session != null) DecodeSession.DeleteSession(ref session);
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
}
