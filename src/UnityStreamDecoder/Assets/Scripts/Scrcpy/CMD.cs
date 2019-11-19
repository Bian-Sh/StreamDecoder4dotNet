using System;
using System.Diagnostics;
using System.Timers;
using UnityEngine;


class CMD
{
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
    private Action<CMD, ExecuteState> callback;

    public string StandardOutStr
    {
        get { return standardOutStr; }
    }
    public string ErrorOutStr
    {
        get { return errorOutStr; }
    }

    private QScrcpy adb;
    public CMD(QScrcpy adb)
    {
        this.adb = adb;
    }
    /// <summary>
    /// 可以调用cmd，也可以调用adb，异步读取
    /// </summary>
    /// <param name="StartFileArg">命令</param>
    public void Execute(string exe, string cmd, Action<CMD, ExecuteState> cb)
    {
        //Debug.Log(System.Threading.Thread.CurrentThread.ManagedThreadId);
        callback = cb;
        try
        {
            System.Diagnostics.Process CmdProcess = new System.Diagnostics.Process();

            //如果使用cmd Arguments前面要加/c  如果使用adb Arguments可以直接复制
            CmdProcess.StartInfo.FileName = exe;      // 命令
            CmdProcess.StartInfo.Arguments = exe == "cmd.exe" ? "/c " + cmd : cmd;      // 参数

            CmdProcess.StartInfo.CreateNoWindow = true;         // 不创建新窗口
            CmdProcess.StartInfo.UseShellExecute = false;
            CmdProcess.StartInfo.RedirectStandardInput = true;  // 重定向输入
            CmdProcess.StartInfo.RedirectStandardOutput = true; // 重定向标准输出
            CmdProcess.StartInfo.RedirectStandardError = true;  // 重定向错误输出
                                                                //CmdProcess.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;

            CmdProcess.OutputDataReceived += new System.Diagnostics.DataReceivedEventHandler(p_OutputDataReceived);
            CmdProcess.ErrorDataReceived += new System.Diagnostics.DataReceivedEventHandler(p_ErrorDataReceived);

            CmdProcess.EnableRaisingEvents = true;                      // 启用Exited事件
            CmdProcess.Exited += new EventHandler(CmdProcess_Exited);   // 注册进程结束事件
           
            CmdProcess.Start();
            CmdProcess.BeginOutputReadLine();
            CmdProcess.BeginErrorReadLine();

            // 如果打开注释，则以同步方式执行命令，此例子中用Exited事件异步执行。
            // CmdProcess.WaitForExit();

            if (callback != null) callback(this, ExecuteState.StartSuccess);
            
        }
        catch (Exception ex)
        {
            //isExit = true;
            UnityEngine.Debug.LogWarning(ex);
            if (callback != null) callback(this, ExecuteState.StartFailed);
        }
        if(adb!=null)
            adb.UpdateEvent += Update;
    }


    private void p_OutputDataReceived(object sender, System.Diagnostics.DataReceivedEventArgs e)
    {
        if (string.IsNullOrEmpty(e.Data.Trim())) return;
        standardOutStr += e.Data.Trim();
        standardOutStr += '\n';
    }

    private void p_ErrorDataReceived(object sender, System.Diagnostics.DataReceivedEventArgs e)
    {
        if (string.IsNullOrEmpty(e.Data.Trim())) return;
        errorOutStr += e.Data.Trim();
        errorOutStr += '\n';
    }
    private void CmdProcess_Exited(object sender, EventArgs e)
    {
        Process process = (Process)sender;
        UnityEngine.Debug.Log("exit code:" + process.ExitCode);
        isExit = true;
        UnityEngine.Debug.LogWarning(errorOutStr);
        UnityEngine.Debug.Log(standardOutStr);
    }

    private void Update()
    {
        if (!isExit) return;
        if (callback != null) callback(this, ExecuteState.Finished);
        adb.UpdateEvent -= Update;
        
    }

}

