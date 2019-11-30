using System;
using UnityEngine;

public class Process
{
    //Unity调用端更新代码
    //C#窗体使用WinForm定时器
    //private event Action UpdateEvent;
    //// Update is called once per frame
    //void Update()
    //{
    //    if (UpdateEvent != null)
    //        UpdateEvent();
    //}
    //public void SetEvent(bool isAdd, System.Action ac)
    //{
    //    if (isAdd) UpdateEvent += ac;
    //    else UpdateEvent -= ac;
    //}

    public enum ExecuteState
    {
        None,
        StartSuccess,
        StartFailed,
        Finished
    }
    private bool isExit = false;
    private string standardOutStr;
    private string errorOutStr;
    private int exitCode;
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

    private Action<bool, Action> setUpdateEvent;
    public System.Diagnostics.Process cmdProcess;
    public Process(Action<bool, Action> callback)
    {
        setUpdateEvent = callback;
    }
    /// <summary>
    /// 启动一个进程
    /// </summary>
    /// <param name="exe">文件名</param>
    /// <param name="cmd">命令</param>
    /// <param name="cb">执行回调</param>
    public void Execute(string exe, string cmd, Action<Process, ExecuteState> cb)
    {
        callback = cb;
        try
        {
            cmdProcess = new System.Diagnostics.Process();

            //如果使用cmd Arguments前面要加/c  如果使用adb Arguments可以直接复制
            cmdProcess.StartInfo.FileName = exe;      // 命令
            cmdProcess.StartInfo.Arguments = exe == "cmd.exe" ? "/c " + cmd : cmd;      // 参数

            cmdProcess.StartInfo.CreateNoWindow = true;         // 不创建新窗口
            cmdProcess.StartInfo.UseShellExecute = false;
            cmdProcess.StartInfo.RedirectStandardInput = true;  // 重定向输入
            cmdProcess.StartInfo.RedirectStandardOutput = true; // 重定向标准输出
            cmdProcess.StartInfo.RedirectStandardError = true;  // 重定向错误输出
                                                                //CmdProcess.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;

            cmdProcess.OutputDataReceived += new System.Diagnostics.DataReceivedEventHandler(OutputDataReceived);
            cmdProcess.ErrorDataReceived += new System.Diagnostics.DataReceivedEventHandler(ErrorDataReceived);

            cmdProcess.EnableRaisingEvents = true;                      // 启用Exited事件
            cmdProcess.Exited += new EventHandler(CmdProcess_Exited);   // 注册进程结束事件

            cmdProcess.Start();
            cmdProcess.BeginOutputReadLine();
            cmdProcess.BeginErrorReadLine();

            // 如果打开注释，则以同步方式执行命令，此例子中用Exited事件异步执行。
            // CmdProcess.WaitForExit();

            if (callback != null) callback(this, ExecuteState.StartSuccess);

        }
        catch (Exception ex)
        {
            Debug.LogWarning(ex);
            if (callback != null) callback(this, ExecuteState.StartFailed);
        }
        setUpdateEvent(true, Update);
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
    private void CmdProcess_Exited(object sender, EventArgs e)
    {
        exitCode = cmdProcess.ExitCode;
        isExit = true;
        cmdProcess.Dispose();
        cmdProcess.Close();
    }

    private void Update()
    {
        if (!isExit) return;
        if (callback != null) callback(this, ExecuteState.Finished);
        setUpdateEvent(false, Update);
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

    }
}

