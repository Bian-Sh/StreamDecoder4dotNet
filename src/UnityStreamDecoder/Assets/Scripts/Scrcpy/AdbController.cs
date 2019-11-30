using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using System.Text.RegularExpressions;

public class AdbController
{

    private string deviceLocal = "/sdcard/scrcpy-server.jar";
    public string adbPath;
    /// <summary>
    /// 获取设备信息
    /// </summary>
    /// <param name="cb">获取完成后的回调</param>
    public void GetDevices(Action<List<string>> cb)
    {
        Execute("devices", (sender, state) =>
        {
            if (state == Process.ExecuteState.StartFailed) Debug.LogWarning("启动失败");
            //else if (state == Process.ExecuteState.StartSuccess) Debug.Log("启动成功");
            else if (state == Process.ExecuteState.Finished)
            {
                //正常结束 
                if (sender.ExitCode == 0)
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
                //异常结束
                else
                {
                    if (!string.IsNullOrEmpty(sender.ErrorOutStr)) Debug.LogWarning(sender.ErrorOutStr);
                    if (!string.IsNullOrEmpty(sender.StandardOutStr)) Debug.Log(sender.StandardOutStr);
                }
            }
        });
    }

    /// <summary>
    /// 关闭ADB
    /// </summary>
    public void AdbStartServer()
    {
        Execute("start-server", (sender, state) =>
        {
            if (state == Process.ExecuteState.StartFailed) Debug.LogWarning("启动失败");
            //else if (state == Process.ExecuteState.StartSuccess) Debug.Log("启动成功");
            else if (state == Process.ExecuteState.Finished)
            {
                //正常结束
                if (sender.ExitCode == 0)
                {
                    Debug.Log("ADB 启动");
                }
                //异常结束
                else
                {
                    if (!string.IsNullOrEmpty(sender.ErrorOutStr)) Debug.LogWarning(sender.ErrorOutStr);
                    if (!string.IsNullOrEmpty(sender.StandardOutStr)) Debug.Log(sender.StandardOutStr);
                }
            }
        });
    }
    /// <summary>
    /// 关闭ADB
    /// </summary>
    public void AdbKillServer()
    {
        Execute("kill-server", (sender, state) =>
        {
            if (state == Process.ExecuteState.StartFailed) Debug.LogWarning("启动失败");
            //else if (state == Process.ExecuteState.StartSuccess) Debug.Log("启动成功");
            else if (state == Process.ExecuteState.Finished)
            {
                //正常结束
                if (sender.ExitCode == 0)
                {
                    Debug.Log("ADB 关闭");
                }
                //异常结束
                else
                {
                    if (!string.IsNullOrEmpty(sender.ErrorOutStr)) Debug.LogWarning(sender.ErrorOutStr);
                    if (!string.IsNullOrEmpty(sender.StandardOutStr)) Debug.Log(sender.StandardOutStr);
                }
            }
        });
    }

    /// <summary>
    /// 获取设备IP
    /// </summary>
    /// <param name="serial"></param>
    /// <param name="cmd"></param>
    /// <param name="cb"></param>
    public void GetDeviceIP(string serial, bool iswifi, Action<string> cb)
    {
        string cmd = "shell ip -f inet addr show " + (iswifi ? "wlan0" : "eth0");
        if (!string.IsNullOrEmpty(serial))
        {
            cmd = "-s " + serial + " " + cmd;
        }
        Execute(cmd, (sender, state) =>
        {
            if (state == Process.ExecuteState.StartFailed) Debug.LogWarning("启动失败");
            //else if (state == Process.ExecuteState.StartSuccess) Debug.Log("启动成功");
            else if (state == Process.ExecuteState.Finished)
            {
                //正常结束
                if (sender.ExitCode == 0)
                {
                    if (sender.StandardOutStr.StartsWith("Device"))
                        Debug.Log(sender.StandardOutStr);

                    if (!string.IsNullOrEmpty(sender.StandardOutStr))
                    {
                        bool isUpdateIp = false;
                        foreach (Match match in Regex.Matches(sender.StandardOutStr, "inet [\\d.]*"))
                        {
                            string[] info = match.Value.Trim().Split(' ');
                            if (info.Length >= 1 && !string.IsNullOrEmpty(info[0]) && !string.IsNullOrEmpty(info[1]))
                            {
                                if (!isUpdateIp) cb(info[1].Trim());
                                isUpdateIp = true;
                                Debug.Log(info[1]);
                            }
                        }
                    }
                }
                //异常结束
                else
                {
                    if (!string.IsNullOrEmpty(sender.ErrorOutStr)) Debug.LogWarning(sender.ErrorOutStr);
                    if (!string.IsNullOrEmpty(sender.StandardOutStr)) Debug.Log(sender.StandardOutStr);
                }

            }
        });
    }

    /// <summary>
    /// 复制QScrcpy到手机
    /// </summary>
    /// <param name="serial"></param>
    /// <param name="local"></param>
    /// <param name="successCb"></param>
    public void PushQScrcpy(string serial, string local, Action<bool> IsSuccessCb)
    {
        string cmd = "push " + local + " " + deviceLocal;
        if (!string.IsNullOrEmpty(serial))
        {
            cmd = "-s " + serial + " " + cmd;
        }

        Execute(cmd, (sender, state) =>
        {
            if (state == Process.ExecuteState.StartFailed)
            {
                if (IsSuccessCb != null) IsSuccessCb(false);
                Debug.LogWarning("启动失败");
            }
            //else if (state == Process.ExecuteState.StartSuccess) Debug.Log("启动成功");
            else if (state == Process.ExecuteState.Finished)
            {
                //正常结束
                if (sender.ExitCode == 0)
                {
                    if (IsSuccessCb != null) IsSuccessCb(true);
                }
                //异常结束
                else
                {
                    if (!string.IsNullOrEmpty(sender.ErrorOutStr)) Debug.LogWarning(sender.ErrorOutStr);
                    if (!string.IsNullOrEmpty(sender.StandardOutStr)) Debug.Log(sender.StandardOutStr);
                    if (IsSuccessCb != null) IsSuccessCb(false);
                }
            }
        });
    }

    /// <summary>
    /// 打开反向代理
    /// </summary>
    /// <param name="serial"></param>
    /// <param name="localPort"></param>
    /// <param name="successCb"></param>
    public void OpenReverseProxy(string serial, int localPort, Action<bool> IsSuccessCb)
    {
        string cmd = string.Format("reverse localabstract:{0} tcp:{1}", QScrcpy.Instance.useQtScrcpy ? "qtscrcpy" : "scrcpy", localPort);
        if (!string.IsNullOrEmpty(serial))
        {
            cmd = "-s " + serial + " " + cmd;
        }
        Execute(cmd, (sender, state) =>
        {
            if (state == Process.ExecuteState.StartFailed)
            {
                if (IsSuccessCb != null) IsSuccessCb(false);
                Debug.LogWarning("启动失败");
            }
            //else if (state == Process.ExecuteState.StartSuccess) Debug.Log("启动成功");
            else if (state == Process.ExecuteState.Finished)
            {
                //正常结束
                if (sender.ExitCode == 0)
                {
                    if (IsSuccessCb != null) IsSuccessCb(true);
                }
                //异常结束
                else
                {
                    if (!string.IsNullOrEmpty(sender.ErrorOutStr)) Debug.LogWarning(sender.ErrorOutStr);
                    if (!string.IsNullOrEmpty(sender.StandardOutStr)) Debug.Log(sender.StandardOutStr);
                    if (IsSuccessCb != null) IsSuccessCb(false);
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
            cmd = "-s " + serial + " " + cmd;
        }
        Execute(cmd, (sender, state) =>
        {
            if (state == Process.ExecuteState.StartFailed)
                Debug.LogWarning("启动失败");
            //else if (state == Process.ExecuteState.StartSuccess) Debug.Log("启动成功");
            else if (state == Process.ExecuteState.Finished)
            {
                //正常结束
                if (sender.ExitCode == 0)
                {
                    Debug.Log("remove QScrcpy Success");
                }
                //异常结束
                else
                {
                    if (!string.IsNullOrEmpty(sender.ErrorOutStr)) Debug.LogWarning(sender.ErrorOutStr);
                    if (!string.IsNullOrEmpty(sender.StandardOutStr)) Debug.Log(sender.StandardOutStr);
                }
            }
        });
    }

    public void CloseReverseProxy(string serial)
    {
        string cmd = string.Format("reverse --remove localabstract:{0}", QScrcpy.Instance.useQtScrcpy ? "qtscrcpy" : "scrcpy");
        if (!string.IsNullOrEmpty(serial))
        {
            cmd = "-s " + serial + " " + cmd;
        }
        Execute(cmd, (sender, state) =>
        {
            if (state == Process.ExecuteState.StartFailed)
                Debug.LogWarning("启动失败");
            //else if (state == Process.ExecuteState.StartSuccess) Debug.Log("启动成功");
            else if (state == Process.ExecuteState.Finished)
            {
                //正常结束
                if (sender.ExitCode == 0)
                {
                    Debug.Log("CloseReverseProxy Success");
                }
                //异常结束
                else
                {
                    if (!string.IsNullOrEmpty(sender.ErrorOutStr)) Debug.LogWarning(sender.ErrorOutStr);
                    if (!string.IsNullOrEmpty(sender.StandardOutStr)) Debug.Log(sender.StandardOutStr);
                }
            }
        });
    }

    /// <summary>
    /// 启动qtscrcpy
    /// </summary>
    /// <param name="serial">设备序列号</param>
    /// <param name="scrWidth">视频宽，比例为16：9</param>
    /// <param name="bitRate">比特率</param>
    /// <returns></returns>
    public Process StartQScrcpyServer(string serial, int scrWidth, int bitRate)
    {
        //width
        //bitrate
        //use "adb forward" instead of "adb reverse"
        //视频裁剪
        //是否发送mp4帧数据
        //安卓端是否接收键鼠控制

        // adb -s P7C0218510000537 shell CLASSPATH=/data/local/tmp/scrcpy-server.jar app_process / com.genymobile.scrcpy.Server 0 8000000 false
        // mark: crop input format: "width:height:x:y" or - for no crop, for example: "100:200:0:0"
        // 这条adb命令是阻塞运行的，m_serverProcess进程不会退出了
        string cmd = "";
        if(QScrcpy.Instance.useQtScrcpy)
        {
            Debug.Log("使用QtScrcpy");
           cmd = string.Format(" shell CLASSPATH=/sdcard/scrcpy-server.jar app_process / com.genymobile.scrcpy.Server {0} {1} false - false true", scrWidth, bitRate);
        }
        else
        {
            Debug.Log("使用Scrcpy");
           cmd = string.Format(" shell CLASSPATH=/sdcard/scrcpy-server.jar app_process / com.genymobile.scrcpy.Server {0} {1} false ", scrWidth, bitRate);
        }
        if (!string.IsNullOrEmpty(serial))
        {
            cmd = "-s " + serial + " " + cmd;
        }
        Process process = new Process();
        process.Execute(adbPath, cmd, (sender, state) =>
        {
            if (state == Process.ExecuteState.StartFailed)
            {
                Debug.LogWarning("QScrcpy启动失败");
            }
            else if (state == Process.ExecuteState.StartSuccess)
            {
                Debug.Log("QScrcpy启动成功");
            }
            else if (state == Process.ExecuteState.Finished)
            {
                //正常结束
                if (sender.ExitCode == 0)
                {
                    Debug.Log("Close QScrcpy Success");
                }
                //异常结束
                else
                {
                    if (!string.IsNullOrEmpty(sender.ErrorOutStr)) Debug.LogWarning(sender.ErrorOutStr);
                    if (!string.IsNullOrEmpty(sender.StandardOutStr)) Debug.Log(sender.StandardOutStr);
                }
            }

        });
        return process;
    }

    /// <summary>
    /// 执行
    /// </summary>
    /// <param name="cmd"></param>
    /// <param name="cb"></param>
    private void Execute(string cmd, Action<Process, Process.ExecuteState> cb)
    {
        new Process().Execute(adbPath, cmd, cb);
    }
}
