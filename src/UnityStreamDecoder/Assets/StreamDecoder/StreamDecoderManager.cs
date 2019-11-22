using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Threading;
using UnityEngine;


namespace SStreamDecoder
{
    public enum OptionType
    {
        DemuxTimeout = 1,
        PushFrameInterval,
        AlwaysWaitBitStream,
        WaitBitStreamTimeout,
    }

    public struct DotNetFrame
    {
        public int playerID;
        public int width;
        public int height;
        public IntPtr frame_y;
        public IntPtr frame_u;
        public IntPtr frame_v;
    };
    public static class StreamDecoder
    {
        private static bool isInit = false;
        public static bool IsInit { get { return isInit; } }
        public static string dllPath = "";

        private static IntPtr avutil_dll;
        private static IntPtr swresample_dll;
        private static IntPtr swscale_dll;
        private static IntPtr avcodec_dll;
        private static IntPtr avformat_dll;

        public static IntPtr streamDecoder_dll;

        /// <summary>
        /// 初始化StreamDecoder 设置日志回调函数 委托
        /// </summary>
        /// <param name="pfun"></param>
        /// <param name="pDraw"></param>
        private delegate void StreamDecoderInitialize(DLL_Debug_Log pfun, DLL_Draw_Frame pDraw);

        /// <summary>
        /// 注销StreamDecoder委托
        /// </summary>
        private delegate void StreamDecoderDeInitialize();

        /// <summary>
        /// 获取版本信息委托
        /// </summary>
        /// <returns></returns>
        private delegate IntPtr GetStreamDecoderVersion();

        /// <summary>
        /// 创建一个Session委托
        /// </summary>
        /// <returns></returns>
        public delegate IntPtr CreateSession(int playerID, int dataCacheSize);

        /// <summary>
        /// 删除一个Session委托
        /// </summary>
        /// <param name="session"></param>
        public delegate void DeleteSession(IntPtr session);

        /// <summary>
        /// 尝试解封装字节流数据委托
        /// </summary>
        /// <param name="session"></param>
        /// <returns></returns>
        public delegate bool TryBitStreamDemux(IntPtr session);

        /// <summary>
        /// 尝试解封装网络流数据委托
        /// </summary>
        /// <param name="session"></param>
        /// <param name="url"></param>
        /// <returns></returns>
        public delegate bool TryNetStreamDemux(IntPtr session, string url);

        /// <summary>
        /// 开始解码委托
        /// </summary>
        /// <param name="session"></param>
        public delegate void BeginDecode(IntPtr session);

        /// <summary>
        /// 停止解码委托
        /// </summary>
        /// <param name="session"></param>
        public delegate void StopDecode(IntPtr session);

        /// <summary>
        /// 获取 数据流缓冲区 可用空间（字节） 委托
        /// </summary>
        /// <param name="session"></param>
        /// <returns></returns>
        public delegate int GetCacheFreeSize(IntPtr session);

        /// <summary>
        /// 向 数据流缓冲区 追加数据 委托
        /// </summary>
        /// <param name="session"></param>
        /// <param name="data"></param>
        /// <param name="len"></param>
        /// <returns></returns>
        public delegate bool PushStream2Cache(IntPtr session, byte[] data, int len);

        /// <summary>
        /// 设置session 配置属性 委托
        /// </summary>
        /// <param name="session"></param>
        /// <param name="type"></param>
        /// <param name="value"></param>
        public delegate void SetOption(IntPtr session, OptionType type, int value);


        

        /// <summary>
        /// C++ 回调函数 Log 委托
        /// </summary>
        /// <param name="level"></param>
        /// <param name="log"></param>
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void DLL_Debug_Log(int level, IntPtr log);

        /// <summary>
        /// C++ 回调函数 绘制图像 委托
        /// </summary>
        /// <param name="frame"></param>
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void DLL_Draw_Frame(DotNetFrame frame);


        public static event Action<int, string> logEvent;
        public static event Action<DotNetFrame> drawEvent;

        #region 加载卸载动态链接库
        public static bool LoadLibrary()
        {
            avutil_dll = Native.LoadLibrary(dllPath + "avutil-55.dll");
            if (avutil_dll == IntPtr.Zero) { Debug.Log("avutil-55.dll 加载失败"); return false; }
            else { Debug.Log("avutil-55.dll 加载成功"); }

            swresample_dll = Native.LoadLibrary(dllPath + "swresample-2.dll");
            if (swresample_dll == IntPtr.Zero) { Debug.Log("swresample-2.dll 加载失败"); return false; }
            else { Debug.Log("swresample-2.dll 加载成功"); }

            swscale_dll = Native.LoadLibrary(dllPath + "swscale-4.dll");
            if (swscale_dll == IntPtr.Zero) { Debug.Log("swscale-4.dll 加载失败"); return false; }
            else { Debug.Log("swscale-4.dll 加载成功"); }

            avcodec_dll = Native.LoadLibrary(dllPath + "avcodec-57.dll");
            if (avcodec_dll == IntPtr.Zero) { Debug.Log("avcodec-57.dll 加载失败"); return false; }
            else { Debug.Log("avcodec-57.dll 加载成功"); }

            avformat_dll = Native.LoadLibrary(dllPath + "avformat-57.dll");
            if (avformat_dll == IntPtr.Zero) { Debug.Log("avformat-57.dll 加载失败"); return false; }
            else { Debug.Log("avformat-57.dll 加载成功"); }

            //qt5core_dll = Native.LoadLibrary(dllPath + "Qt5Core.dll");
            //if (qt5core_dll == IntPtr.Zero) { Debug.Log("Qt5Core.dll 加载失败"); return false; }
            //else { Debug.Log("Qt5Core.dll 加载成功"); }

            //qt5cored_dll = Native.LoadLibrary(dllPath + "Qt5Cored.dll");
            //if (qt5cored_dll == IntPtr.Zero) { Debug.Log("Qt5Cored.dll 加载失败"); return false; }
            //else { Debug.Log("Qt5Cored.dll 加载成功"); }

          

            streamDecoder_dll = Native.LoadLibrary(dllPath + "StreamDecoder.dll");
            if (streamDecoder_dll == IntPtr.Zero) { Debug.Log("StreamDecoder.dll 加载失败"); return false; }
            else { Debug.Log("StreamDecoder.dll 加载成功"); }
            return true;
        }
        public static void FreeLibrary()
        {
            if (streamDecoder_dll != IntPtr.Zero) Debug.Log(Native.FreeLibrary(streamDecoder_dll) ? "StreamDecoder.dll 卸载成功" : "StreamDecoder.dll 卸载失败");
            //if (qt5cored_dll != IntPtr.Zero) Debug.Log(Native.FreeLibrary(qt5cored_dll) ? "Qt5Cored.dll 卸载成功" : "Qt5Cored.dll 卸载失败");
            //if (qt5core_dll != IntPtr.Zero) Debug.Log(Native.FreeLibrary(qt5core_dll) ? "Qt5Core.dll 卸载成功" : "Qt5Core.dll 卸载失败");
            if (avformat_dll != IntPtr.Zero) Debug.Log(Native.FreeLibrary(avformat_dll) ? "avformat-57.dll 卸载成功" : "avformat-57.dll 卸载失败");
            if (avcodec_dll != IntPtr.Zero) Debug.Log(Native.FreeLibrary(avcodec_dll) ? "avcodec-57.dll 卸载成功" : "avcodec-57.dll 卸载失败");
            if (swscale_dll != IntPtr.Zero) Debug.Log(Native.FreeLibrary(swscale_dll) ? "swscale-4.dll 卸载成功" : "swscale-4.dll 卸载失败");
            if (swresample_dll != IntPtr.Zero) Debug.Log(Native.FreeLibrary(swresample_dll) ? "swresample-2.dll 卸载成功" : "swresample-2.dll 卸载失败");
            if (avutil_dll != IntPtr.Zero) Debug.Log(Native.FreeLibrary(avutil_dll) ? "avutil-55.dll 卸载成功" : "avutil-55.dll 卸载失败");
        }
        #endregion

        /// <summary>
        /// 必须调用的初始化函数
        /// </summary>
        public static void InitStreamDecoder()
        {
            isInit = true;
            DLL_Debug_Log log = StreamDecoderLog;
            DLL_Draw_Frame draw = OnDrawFrame;
            Native.Invoke<StreamDecoderInitialize>(streamDecoder_dll, log, draw);
        }

        /// <summary>
        /// 清理函数
        /// </summary>
        public static void DeInitStreamDecoder()
        {
            Native.Invoke<StreamDecoderDeInitialize>(streamDecoder_dll);
        }

        /// <summary>
        /// C++ 动态库回调函数
        /// </summary>
        /// <param name="level"></param>
        /// <param name="log"></param>
        private static void StreamDecoderLog(int level, IntPtr log)
        {
            //string logStr = ;
            string _log = "<b>" + Marshal.PtrToStringAnsi(log) + "</b>";
            
            if(logEvent == null)
            {
                if (level == 0) Debug.Log(_log);
                else if (level == 1) Debug.LogWarning(_log);
                else Debug.LogError(_log);
            }
            else
            {
                logEvent(level, _log);
            }
        }

        private static void OnDrawFrame(DotNetFrame frame)
        {
            if (drawEvent == null) return;
            drawEvent(frame);
        }


        /// <summary>
        /// 获取版本信息
        /// </summary>
        /// <returns></returns>
        public static string StreamDecoderVersion()
        {
            IntPtr versionPtr = Native.Invoke<IntPtr, GetStreamDecoderVersion>(streamDecoder_dll);
            return Marshal.PtrToStringAnsi(versionPtr);
        }

        //private delegate int Test();
        //public static int Count()
        //{
        //    return Native.Invoke<int, Test>(streamDecoder_dll);
        //}

    }
}

