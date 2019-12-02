using System;
using System.Linq;
using System.Text;

/// <summary>
/// Windows上使用的是小端模式，发送给安卓要用大端模式
/// </summary>
public class AssemblyControlCmd {

    private enum EType
    {
        Keycode = 0,
        Text,
        Mouse,
        Scroll,
        Command,
        Touch
    }
    /// <summary>
    /// 拼装键盘事件
    /// </summary>
    /// <param name="akeAction">事件类型</param>
    /// <param name="akeycode">键盘值</param>
    /// <param name="ametastate">辅助值</param>
    /// <returns></returns>
	public static byte[] Ass_KeyCode(AndroidKeyeventAction akeAction, AndroidKeycode akeycode, AndroidMetastate ametastate)
    {
        byte[] cmdBytes = new byte[10];
        //type
        cmdBytes[0] = (byte)EType.Keycode;
        cmdBytes[1] = (byte)akeAction;
        byte[] akeyCodeBytes = BitConverter.GetBytes((int)akeycode);
        cmdBytes[2] = akeyCodeBytes[3];
        cmdBytes[3] = akeyCodeBytes[1];
        cmdBytes[4] = akeyCodeBytes[2];
        cmdBytes[5] = akeyCodeBytes[0];
        byte[] ametstateBytes = BitConverter.GetBytes((int)ametastate);
        cmdBytes[6] = ametstateBytes[3];
        cmdBytes[7] = ametstateBytes[2];
        cmdBytes[8] = ametstateBytes[1];
        cmdBytes[9] = ametstateBytes[0];
        return cmdBytes;
    }

    /// <summary>
    /// 拼装Text消息
    /// 中文发送失败
    /// </summary>
    /// <param name="msg"></param>
    /// <returns></returns>
    public static byte[] Ass_Text(string msg)
    {
        byte[] type = { (byte)EType.Text };
        byte[] msgBytes = Encoding.Default.GetBytes(msg);
        byte[] lenBytes = BitConverter.GetBytes((short)msgBytes.Length);
        byte[] cmdBytes = type.Concat(lenBytes).Concat(msgBytes).ToArray();
        cmdBytes[1] = lenBytes[1];
        cmdBytes[2] = lenBytes[0];
        return cmdBytes;
    }

    public static byte[] Ass_Mouse(AndroidMotioneventAction amAction, AndroidMotioneventButtons amButtons, short mousePosX, short mousePosY, short width, short height)
    {
        byte[] cmdBytes = new byte[14];
        //type
        cmdBytes[0] = (byte)EType.Mouse;
        cmdBytes[1] = (byte)amAction;

        byte[] motionevent = BitConverter.GetBytes((int)amButtons);
        cmdBytes[2] = motionevent[3];
        cmdBytes[3] = motionevent[2];
        cmdBytes[4] = motionevent[1];
        cmdBytes[5] = motionevent[0];

        byte[] fixedX = BitConverter.GetBytes(mousePosX);
        cmdBytes[6] = fixedX[1];
        cmdBytes[7] = fixedX[0];

        byte[] fixedY = BitConverter.GetBytes(mousePosY);
        cmdBytes[8] = fixedY[1];
        cmdBytes[9] = fixedY[0];


        byte[] deviceCrtW = BitConverter.GetBytes(width);
        cmdBytes[10] = deviceCrtW[1];
        cmdBytes[11] = deviceCrtW[0];

        byte[] deviceCrtH = BitConverter.GetBytes(height);
        cmdBytes[12] = deviceCrtH[1];
        cmdBytes[13] = deviceCrtH[0];

        return cmdBytes;
 
    }

    public static byte[] Ass_Scroll(short mousePosX, short mousePosY, short width, short height, float hScroll, float vScroll)
    {
        byte[] cmdBytes = new byte[17];
        cmdBytes[0] = (byte)EType.Scroll;

        byte[] fixedX = BitConverter.GetBytes(mousePosX);
        cmdBytes[1] = fixedX[1];
        cmdBytes[2] = fixedX[0];

        byte[] fixedY = BitConverter.GetBytes(mousePosY);
        cmdBytes[3] = fixedY[1];
        cmdBytes[4] = fixedY[0];


        byte[] deviceCrtW = BitConverter.GetBytes(width);
        cmdBytes[5] = deviceCrtW[1];
        cmdBytes[6] = deviceCrtW[0];

        byte[] deviceCrtH = BitConverter.GetBytes(height);
        cmdBytes[7] = deviceCrtH[1];
        cmdBytes[8] = deviceCrtH[0];

        byte[] hscrollBytes = BitConverter.GetBytes(hScroll);
        cmdBytes[9] = hscrollBytes[3];
        cmdBytes[10] = hscrollBytes[2];
        cmdBytes[11] = hscrollBytes[1];
        cmdBytes[12] = hscrollBytes[0];

        byte[] vscrollBytes = BitConverter.GetBytes(vScroll);
        cmdBytes[13] = vscrollBytes[3];
        cmdBytes[14] = vscrollBytes[2];
        cmdBytes[15] = vscrollBytes[1];
        cmdBytes[16] = vscrollBytes[0];
        return cmdBytes;
    }

    public static void Ass_Command()
    {

    }

    public static byte[] Ass_Touch(byte touchId, AndroidMotioneventAction amAction, short mousePosX, short mousePosY, short width, short height)
    {
        byte[] cmdBytes = new byte[11];
        cmdBytes[0] = (byte)EType.Touch;
        cmdBytes[1] = touchId;
        cmdBytes[2] = (byte)amAction;

        byte[] fixedX = BitConverter.GetBytes(mousePosX);
        cmdBytes[3] = fixedX[1];
        cmdBytes[4] = fixedX[0];

        byte[] fixedY = BitConverter.GetBytes(mousePosY);
        cmdBytes[5] = fixedY[1];
        cmdBytes[6] = fixedY[0];


        byte[] deviceCrtW = BitConverter.GetBytes(width);
        cmdBytes[7] = deviceCrtW[1];
        cmdBytes[8] = deviceCrtW[0];

        byte[] deviceCrtH = BitConverter.GetBytes(height);
        cmdBytes[9] = deviceCrtH[1];
        cmdBytes[10] = deviceCrtH[0];

        return cmdBytes;
    }
}
