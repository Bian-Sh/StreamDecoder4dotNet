using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Threading;
using UnityEngine;
using UnityEngine.UI;

namespace SStreamDecoder
{
    public enum OptionType
    {
        DataCacheSize = 1,
        DemuxTimeout,
        PushFrameInterval,
        AlwaysWaitBitStream,
        WaitBitStreamTimeout,
        AutoDecode,
        DecodeThreadCount,
    }
    public enum SessionEventType
    {
        DemuxSuccess = 1,
    }
    public struct Frame
    {
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
        private delegate void StreamDecoderInitialize(DLL_Debug_Log pLog);

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
        public delegate IntPtr CreateSession(int playerID);

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
        public delegate void TryBitStreamDemux(IntPtr session);

        /// <summary>
        /// 尝试解封装网络流数据委托
        /// </summary>
        /// <param name="session"></param>
        /// <param name="url"></param>
        /// <returns></returns>
        public delegate void TryNetStreamDemux(IntPtr session, string url);

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


        public delegate void SetSessionEvent(IntPtr session, DLL_Decode_Event sessionEvent, DLL_Draw_Frame drawEvent);


        #region C++ 回调委托
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
        public delegate void DLL_Draw_Frame(Frame frame);

        /// <summary>
        /// C++ 回调函数 解码时间 委托
        /// </summary>
        /// <param name="playerID"></param>
        /// <param name="eventType"></param>
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void DLL_Decode_Event(int playerID, int eventType);
        #endregion


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

            streamDecoder_dll = Native.LoadLibrary(dllPath + "StreamDecoder.dll");
            if (streamDecoder_dll == IntPtr.Zero) { Debug.Log("StreamDecoder.dll 加载失败"); return false; }
            else { Debug.Log("StreamDecoder.dll 加载成功"); }
            return true;
        }
        public static void FreeLibrary()
        {
            if (streamDecoder_dll != IntPtr.Zero) Debug.Log(Native.FreeLibrary(streamDecoder_dll) ? "StreamDecoder.dll 卸载成功" : "StreamDecoder.dll 卸载失败");
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
            Native.Invoke<StreamDecoderInitialize>(streamDecoder_dll, log);
        }

        /// <summary>
        /// 清理函数
        /// </summary>
        public static void DeInitStreamDecoder()
        {
            Native.Invoke<StreamDecoderDeInitialize>(streamDecoder_dll);
        }

        /// <summary>
        /// C++ 动态库回调函数, 打印Log
        /// </summary>
        /// <param name="level"></param>
        /// <param name="_log"></param>
        private static void StreamDecoderLog(int level, IntPtr _log)
        {
            string log = "<b>" + Marshal.PtrToStringAnsi(_log) + "</b>";
            Debug.Log(log);
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

        private static List<int> decoderID = new List<int>();
        public static int GetNewID()
        {
            int id = 0;
            while(decoderID.Contains(id))
            {
                id++;
            }
            decoderID.Add(id);
            return id;
        }
    }
}

