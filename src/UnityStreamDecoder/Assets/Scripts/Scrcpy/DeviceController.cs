using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.EventSystems;

public class DeviceController : MonoBehaviour, IPointerDownHandler, IPointerUpHandler
{

    public void OnPointerDown(PointerEventData eventData)
    {
        Vector2Int fixedPos = GetDeviceFixedPos(CurrMousePosition());
        Debug.Log(fixedPos);
        byte[] info = new byte[14];
        info[0] = (byte)2;
        info[1] = (byte)D_Input.AndroidMotioneventAction.AMOTION_EVENT_ACTION_DOWN;
        //AndroidMotioneventButtons ,
        byte[] motionevent = System.BitConverter.GetBytes((int)D_Input.AndroidMotioneventButtons.AMOTION_EVENT_BUTTON_PRIMARY);
        info[2] = motionevent[3];
        info[3] = motionevent[2];
        info[4] = motionevent[1];
        info[5] = motionevent[0];


        byte[] fixedX = System.BitConverter.GetBytes((short)fixedPos.x);
        info[6] = fixedX[1];
        info[7] = fixedX[0];

        byte[] fixedY = System.BitConverter.GetBytes((short)fixedPos.y);
        info[8] = fixedY[1];
        info[9] = fixedY[0];

        
        byte[] deviceCrtW = System.BitConverter.GetBytes((short)QScrcpy.Instance.Width);
        info[10] = deviceCrtW[1];
        info[11] = deviceCrtW[0];

        byte[] deviceCrtH = System.BitConverter.GetBytes((short)QScrcpy.Instance.Height);
        info[12] = deviceCrtH[1];
        info[13] = deviceCrtH[0];

        try
        {
            QScrcpy.Instance.client.Send(info);
        }
        catch (System.Exception ex)
        {
            System.Console.WriteLine(ex);
        }
    }
    public void OnPointerUp(PointerEventData eventData)
    {
        Vector2Int fixedPos = GetDeviceFixedPos(CurrMousePosition());
        Debug.Log(fixedPos);
        byte[] info = new byte[14];
        info[0] = (byte)2;
        info[1] = (byte)D_Input.AndroidMotioneventAction.AMOTION_EVENT_ACTION_UP;
        //AndroidMotioneventButtons ,
        byte[] motionevent = System.BitConverter.GetBytes((int)D_Input.AndroidMotioneventButtons.AMOTION_EVENT_BUTTON_PRIMARY);
        info[2] = motionevent[3];
        info[3] = motionevent[2];
        info[4] = motionevent[1];
        info[5] = motionevent[0];


        byte[] fixedX = System.BitConverter.GetBytes((short)fixedPos.x);
        info[6] = fixedX[1];
        info[7] = fixedX[0];

        byte[] fixedY = System.BitConverter.GetBytes((short)fixedPos.y);
        info[8] = fixedY[1];
        info[9] = fixedY[0];


        byte[] deviceCrtW = System.BitConverter.GetBytes((short)QScrcpy.Instance.Width);
        info[10] = deviceCrtW[1];
        info[11] = deviceCrtW[0];

        byte[] deviceCrtH = System.BitConverter.GetBytes((short)QScrcpy.Instance.Height);
        info[12] = deviceCrtH[1];
        info[13] = deviceCrtH[0];

        try
        {
            QScrcpy.Instance.client.Send(info);
        }
        catch (System.Exception ex)
        {
            System.Console.WriteLine(ex);
        }
    }

    //获取相对于手机的屏幕坐标的位置
    private Vector2Int GetDeviceFixedPos(Vector2 mousePos)
    {
        Vector2 currentPoint = CurrMousePosition();
        Vector2 canvasSize = (transform as RectTransform).sizeDelta;
        Vector2 imgPos = new Vector2(currentPoint.x + canvasSize.x / 2, currentPoint.y + canvasSize.y / 2);
        imgPos.y = canvasSize.y - imgPos.y;
        Vector2 imgPosRate = new Vector2(imgPos.x / canvasSize.x, imgPos.y / canvasSize.y);
        Vector2Int finalyFixedPos = new Vector2Int((int)(imgPosRate.x * QScrcpy.Instance.Width), (int)(imgPosRate.y * QScrcpy.Instance.Height));
        return finalyFixedPos;
    }

    private Vector2 CurrMousePosition()
    {
        Vector2 vecMouse;
        RectTransformUtility.ScreenPointToLocalPointInRectangle(transform as RectTransform, Input.mousePosition, null, out vecMouse);
        return vecMouse;
    }
}
