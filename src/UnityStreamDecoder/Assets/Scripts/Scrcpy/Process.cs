using System;
using System.Collections.Generic;
using System.Threading;
using UnityEngine;

public class Process
{
    private struct ThreadParam
    {
        public string exe;
        public string cmd;
    }

    public enum ExecuteState
    {
        None,
        StartSuccess,
        StartFailed,
        Finished
    }
    private List<ExecuteState> cmdList = new List<ExecuteState>();
    private bool isExit = false;
    private string standardOutStr;
    private string errorOutStr;
    private int exitCode;
    //同步回调
    private Action<Process, ExecuteState> callback;

    public string StandardOutStr
    {
        get { return standardOutStr; }
    }
    public string ErrorOutStr
    {
        get { return errorOutStr; }
    }
    public int ExitCode
    {
        get { return exitCode; }
    }


    public System.Diagnostics.Process cmdProcess;
    public Process()
    {
        Scrcpy.Instance.UpdateEvent += Update;
    }
    /// <summary>
    /// 启动一个进程
    /// </summary>
    /// <param name="exe">文件名</param>
    /// <param name="cmd">命令</param>
    /// <param name="cb">执行回调</param>
    public void Execute(string exe, string cmd, Action<Process, ExecuteState> cb)
    {
        new Thread(new ParameterizedThreadStart(AsyncExecute)).Start(new ThreadParam { exe = exe, cmd = cmd });
        callback = cb;
      
    }
    private void AsyncExecute(object obj)
    {
        
        ThreadParam para = (ThreadParam)obj;
        try
        {
            cmdProcess = new System.Diagnostics.Process();

            //如果使用cmd Arguments前面要加/c  如果使用adb Arguments可以直接复制
            cmdProcess.StartInfo.FileName = para.exe;      // 命令
            cmdProcess.StartInfo.Arguments = para.exe == "cmd.exe" ? "/c " + para.cmd : para.cmd;      // 参数

            cmdProcess.StartInfo.CreateNoWindow = true;         // 不创建新窗口
            cmdProcess.StartInfo.UseShellExecute = false;
            cmdProcess.StartInfo.RedirectStandardInput = true;  // 重定向输入
            cmdProcess.StartInfo.RedirectStandardOutput = true; // 重定向标准输出
            cmdProcess.StartInfo.RedirectStandardError = true;  // 重定向错误输出
                                                                //CmdProcess.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;

            cmdProcess.OutputDataReceived += new System.Diagnostics.DataReceivedEventHandler(OutputDataReceived);
            cmdProcess.ErrorDataReceived += new System.Diagnostics.DataReceivedEventHandler(ErrorDataReceived);

            ////在Unity发布后Exited不会调用，原因未知
            //cmdProcess.EnableRaisingEvents = true;                      // 启用Exited事件
            //cmdProcess.Exited += new EventHandler(CmdProcess_Exited);   // 注册进程结束事件

            cmdProcess.Start();
            cmdProcess.BeginOutputReadLine();
            cmdProcess.BeginErrorReadLine();

            if (callback != null) PushToMainThread(ExecuteState.StartSuccess);
        }
        catch (Exception ex)
        {
            Debug.LogWarning(ex);
            if (callback != null) PushToMainThread(ExecuteState.StartFailed);
        }


        //等待执行完毕
        cmdProcess.WaitForExit();

        exitCode = cmdProcess.ExitCode;
        if (callback != null) PushToMainThread(ExecuteState.Finished);
    }


    private void OutputDataReceived(object sender, System.Diagnostics.DataReceivedEventArgs e)
    {
        if (string.IsNullOrEmpty(e.Data.Trim())) return;
        standardOutStr += e.Data.Trim();
        standardOutStr += '\n';
    }

    private void ErrorDataReceived(object sender, System.Diagnostics.DataReceivedEventArgs e)
    {
        if (string.IsNullOrEmpty(e.Data.Trim())) return;
        errorOutStr += e.Data.Trim();
        errorOutStr += '\n';
    }


    private void Update()
    {

        if (cmdList.Count <= 0) return;

        lock(cmdList)
        {
            int size = cmdList.Count;
            for (int i = 0; i < size; i++)
            {
                callback(this, cmdList[i]);
                if(cmdList[i] == ExecuteState.Finished)
                {
                    isExit = true;
                    Scrcpy.Instance.UpdateEvent -= Update;
                }
            }
            cmdList.RemoveRange(0, size);
        }
    }

    public void Kill()
    {
        if (isExit) return;
        try
        {
            cmdProcess.Kill();
            cmdProcess.Dispose();
            cmdProcess.Close();
        }
        catch (Exception ex)
        {
            Debug.LogWarning(ex);
        }
        Scrcpy.Instance.UpdateEvent -= Update;

    }

    private void PushToMainThread(ExecuteState state)
    {
        lock(cmdList)
        {
            cmdList.Add(state);
        }
    }
}

