using System;
using System.Runtime.InteropServices;
using UnityEngine;
public class Native
{
    /// <summary>
    /// 调用Dll有返回值方法
    /// <summary>
    /// <typeparam name="T">返回值<typeparam>
    /// <typeparam name="T2">与Dll中函数签名保持一致的自定义委托类型<typeparam>
    /// <param name="library">链接库指针<param>
    /// <param name="pars">参数列表<param>
    /// <returns>返回值<returns>
    public static T Invoke<T, T2>(IntPtr library, params object[] pars)
    {
        //获取函数地址
        IntPtr funPtr = GetProcAddress(library, typeof(T2).Name);
        if (funPtr == IntPtr.Zero)
        {
            Debug.LogWarning("Could not gain reference to method address...");
            return default(T);
        }
        var func = Marshal.GetDelegateForFunctionPointer(funPtr, typeof(T2));
        return (T)func.DynamicInvoke(pars);
    }
    /// <summary>
    /// 调用Dll无返回值方法
    /// <summary>
    /// <typeparam name="T">与Dll中函数签名保持一致的自定义委托类型<typeparam>
    /// <param name="library">链接库指针<param>
    /// <param name="pars">参数列表<param>
    public static void Invoke<T>(IntPtr library, params object[] pars)
    {
        //获取函数地址
        IntPtr funPtr = GetProcAddress(library, typeof(T).Name);
        if (funPtr == IntPtr.Zero)
        {
            Debug.LogWarning("Could not gain reference to method address...");
            return;
        }
        var func = Marshal.GetDelegateForFunctionPointer(funPtr, typeof(T));
        func.DynamicInvoke(pars);
    }

    [DllImport("kernel32", SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool FreeLibrary(IntPtr hModule);

    [DllImport("kernel32", SetLastError = true, CharSet = CharSet.Unicode)]
    public static extern IntPtr LoadLibrary(string lpFileName);

    [DllImport("kernel32")]
    public static extern IntPtr GetProcAddress(IntPtr hModule, string procedureName);
}