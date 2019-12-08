using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace TestDecoder_CSharp
{
   
  
    class ExternMethod
    {
        public const string dllPath = "StreamDecoder.dll";

        [DllImport("kernel32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool AllocConsole();

        [DllImport("Kernel32")]
        public static extern void FreeConsole();


        /// <summary>
        /// C++ 回调函数 Log 委托
        /// </summary>
        /// <param name="level"></param>
        /// <param name="log"></param>
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void DLL_Debug_Log(int level, IntPtr log);


        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr GetStreamDecoderVersion();

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl)]
        public static extern void StreamDecoderInitialize(DLL_Debug_Log pLog);

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr CreateSession(int playerID);

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr TestGUID(char[] ptr);
    }
}
