using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Threading;
using UnityEngine;


namespace StreamDecoderManager
{
    public struct DotNetFrame
    {
        public int width;
        public int height;
        public IntPtr frame_y;
        public IntPtr frame_u;
        public IntPtr frame_v;
    };
    public static class StreamDecoder
    {
        public static string dllPath = "";

        private static IntPtr avutil_dll;
        private static IntPtr swresample_dll;
        private static IntPtr swscale_dll;
        private static IntPtr avcodec_dll;
        private static IntPtr avformat_dll;
        //private static IntPtr qt5core_dll;
        //private static IntPtr qt5cored_dll;

        public static IntPtr streamDecoder_dll;

        private delegate void StreamDecoderInitialize(DLL_Debug_Log pfun, DLL_Draw_Frame pDraw);
        private delegate void SetPushFrameInterval(int wait);
        private delegate void StreamDecoderDeInitialize();

        public delegate IntPtr GetStreamDecoderVersion();

        public delegate IntPtr CreateSession();

        public delegate void DeleteSession(IntPtr session);

        public delegate bool OpenDemuxThread(IntPtr session, int waitDemuxTime);


        public delegate void BeginDecode(IntPtr session);

        public delegate void StopDecode(IntPtr session);

        public delegate int GetCacheFreeSize(IntPtr session);

        public delegate bool PushStream2Cache(IntPtr session, byte[] data, int len);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void DLL_Debug_Log(int level, IntPtr log);

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

        //使用前初始化
        public static void InitializeStreamDecoder()
        {
            DLL_Debug_Log log = StreamDecoderLog;
            DLL_Draw_Frame draw = OnDrawFrame;
            Native.Invoke<StreamDecoderInitialize>(streamDecoder_dll, log, draw);
        }
        public static void SetStreamDecoderPushFrameInterval(int wait)
        {
            Native.Invoke<SetPushFrameInterval>(streamDecoder_dll, wait);
        }
        public static void DeInitializeStreamDecoder()
        {
            Native.Invoke<StreamDecoderDeInitialize>(streamDecoder_dll);
        }

        /// <summary>
        /// C++ 动态库回调函数
        /// </summary>
        /// <param name="level"></param>
        /// <param name="log"></param>
        public static void StreamDecoderLog(int level, IntPtr log)
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

        public static void OnDrawFrame(DotNetFrame frame)
        {
            if (drawEvent == null) return;
            drawEvent(frame);
        }

    }
}

