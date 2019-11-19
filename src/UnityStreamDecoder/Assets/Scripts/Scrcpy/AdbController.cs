using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using System.Text.RegularExpressions;

public class AdbController : MonoBehaviour {


    /// <summary>
    /// 获取设备信息
    /// </summary>
    /// <param name="cb">获取完成后的回调</param>
    public void GetDevices(Action<List<string>> cb)
    {
        new CMD(QScrcpy.Instance).Execute("adb", "devices", (sender, state) =>
        {
            if (state == CMD.ExecuteState.StartFailed) Debug.LogWarning("启动失败");
            else if (state == CMD.ExecuteState.Finished)
            {
                string[] strsp = sender.StandardOutStr.Split('\n');
                List<string> list = new List<string>();
                for (int i = 0; i < strsp.Length; i++)
                {
                    string[] line = strsp[i].Trim().Split('\t');
                    if (line.Length == 2 && line[1] == "device")
                    {
                        list.Add(line[0]);
                    }
                }
                cb(list);
            }
        });
    }

    /// <summary>
    /// 关闭ADB
    /// </summary>
    public void AdbKillServer()
    {
        new CMD(QScrcpy.Instance).Execute("adb", "kill-server", null);
    }
    
    /// <summary>
    /// 获取设备IP
    /// </summary>
    /// <param name="serial"></param>
    /// <param name="cmd"></param>
    /// <param name="cb"></param>
    public void GetDeviceIP(string serial, bool iswifi, Action<string> cb)
    {
        string cmd = " shell ip -f inet addr show " + (iswifi ? "wlan0" : "eth0");
        if (!string.IsNullOrEmpty(serial))
        {
            cmd = " -s " + serial  + cmd;
        }
        new CMD(QScrcpy.Instance).Execute("adb", cmd, (sender, state) =>
        {
            if (state == CMD.ExecuteState.StartFailed) Debug.LogWarning("启动失败");
            else if (state == CMD.ExecuteState.Finished)
            {
                Debug.LogWarning(sender.ErrorOutStr);
                if (!string.IsNullOrEmpty(sender.StandardOutStr))
                {
                    string[] info = Regex.Match(sender.StandardOutStr, "inet [\\d.]*").Value.Trim().Split(' ');
                    if (info.Length >= 1 && !string.IsNullOrEmpty(info[0]) && !string.IsNullOrEmpty(info[1]))
                    {
                        cb(info[1].Trim());
                    }
                }
            }
        });
    }

    public void TryScrcpy()
    {

    }

    /// <summary>
    /// 复制QScrcpy到手机
    /// </summary>
    /// <param name="serial"></param>
    /// <param name="local"></param>
    /// <param name="successCb"></param>
    public void PushQScrcpy(string serial, string local,  Action successCb)
    {
        string cmd = " push " + local + " /sdcard/scrcpy-server.jar";
        if (!string.IsNullOrEmpty(serial))
        {
            cmd = " -s " + serial + cmd;
        }
        new CMD(QScrcpy.Instance).Execute("adb", cmd, (sender, state) =>
        {
            if (state == CMD.ExecuteState.StartFailed) Debug.LogWarning("启动失败");
            else if (state == CMD.ExecuteState.Finished)
            {
                Debug.LogWarning(sender.ErrorOutStr);
                if(sender.StandardOutStr.StartsWith("adb:"))
                {
                    Debug.Log(sender.StandardOutStr);
                }
                else
                {
                    if (successCb != null) successCb();
                }
            }
        });
    }

    /// <summary>
    /// 移除QScrcpy
    /// </summary>
    /// <param name="serial"></param>
    public void RemoveQScrcpy(string serial)
    {
        string cmd = " shell rm /sdcard/scrcpy-server.jar";
        if (!string.IsNullOrEmpty(serial))
        {
            cmd = " -s " + serial + cmd;
        }
        new CMD(QScrcpy.Instance).Execute("adb", cmd, (sender, state) =>
        {
            if (state == CMD.ExecuteState.StartFailed) Debug.LogWarning("启动失败");
            else if (state == CMD.ExecuteState.Finished)
            {
                Debug.LogWarning(sender.ErrorOutStr);
                Debug.Log(sender.StandardOutStr);
            }
        });
    }

    public void OpenReverseProxy(string serial, int localPort, Action successCb)
    {
        string cmd = " reverse localabstract:scrcpy tcp:" + localPort;
        if (!string.IsNullOrEmpty(serial))
        {
            cmd = " -s " + serial + cmd;
        }
        new CMD(QScrcpy.Instance).Execute("adb", cmd, (sender, state) =>
        {
            if (state == CMD.ExecuteState.StartFailed) Debug.LogWarning("启动失败");
            else if (state == CMD.ExecuteState.Finished)
            {
               
                if(sender.ErrorOutStr == null && sender.StandardOutStr == null)
                {
                    successCb();
                }
                else
                {
                    Debug.LogWarning(sender.ErrorOutStr);
                    Debug.Log(sender.StandardOutStr);
                }
            }
        });
    }

    public void CloseReverseProxy(string serial)
    {
        string cmd = " reverse --remove localabstract:scrcpy";
        if (!string.IsNullOrEmpty(serial))
        {
            cmd = " -s " + serial + cmd;
        }
        new CMD(QScrcpy.Instance).Execute("adb", cmd, null);
    }

    public void StartQScrcpyServer(string serial, int scrWidth, int bitRate)
    {
        string cmd = string.Format(" shell CLASSPATH=/sdcard/scrcpy-server.jar app_process / com.genymobile.scrcpy.Server {0} {1} false ", scrWidth, bitRate);
        if (!string.IsNullOrEmpty(serial))
        {
            cmd = " -s " + serial + cmd;
        }
        new CMD(QScrcpy.Instance).Execute("adb", cmd, (sender, state) =>
        {
            if (state == CMD.ExecuteState.StartSuccess)
            {
                Debug.Log("启动Scrcpy成功");
            }
        });
    }
}
