using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
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
        
        public MainWindow()
        {
            InitializeComponent();

            LogDelegate = Log;
            ExternMethod.StreamDecoderInitialize(LogDelegate);

            IntPtr msgPtr = ExternMethod.GetStreamDecoderVersion();
            Console.WriteLine(Marshal.PtrToStringAnsi(msgPtr));
            
        }
        private static void Log(int level, IntPtr log)
        {
            Console.WriteLine(Marshal.PtrToStringAnsi(log));
        }

        private void CreateSession_Click(object sender, RoutedEventArgs e)
        {

            IntPtr cptr = new IntPtr(100);
            IntPtr ptr = ExternMethod.TestSetObj(cptr);

            Console.WriteLine(ptr);
            
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
