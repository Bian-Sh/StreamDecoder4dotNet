using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace TestDecoder_CSharp
{
    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class MainWindow : Window
    {
        private IntPtr player = IntPtr.Zero;
        ExternMethod.DLL_Debug_Log LogDelegate;

        public static long GetTimestamp()
        {
            //获取从1970年一月一日0点0分0秒0微妙开始
            return (DateTime.UtcNow.Ticks - new DateTime(1970, 1, 1, 0, 0, 0, 0).Ticks) / 10;
        }

        public MainWindow()
        {
            InitializeComponent();

            LogDelegate = Log;
            ExternMethod.StreamDecoderInitialize(LogDelegate);

            IntPtr msgPtr = ExternMethod.GetStreamDecoderVersion();
            Console.WriteLine(Marshal.PtrToStringAnsi(msgPtr));

            long t = GetTimestamp();

            for (int i = 0; i < 1000; i++)
            {
                Guid guid = Guid.NewGuid();

                var v = guid.ToString().ToArray();

                IntPtr ptr = ExternMethod.TestGUID(v);

                Guid _guid;
                if (Guid.TryParse(Marshal.PtrToStringAnsi(ptr), out _guid))
                {
                    //Console.WriteLine("yes");
                }
            }

            Console.WriteLine(GetTimestamp() - t);

        }
        private static void Log(int level, IntPtr log)
        {
            Console.WriteLine(Marshal.PtrToStringAnsi(log));
        }

        private void CreateSession_Click(object sender, RoutedEventArgs e)
        {

           
            
        }
        public string name = "stt";
        private void DeleteSession_Click(object sender, RoutedEventArgs e)
        {
            Console.WriteLine("DeleteSession_Click");
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct TestStruct
    {
        public int id;
        public string name;
    }
}
