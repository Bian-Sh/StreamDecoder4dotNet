using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.EventSystems;

public class DeviceController : MonoBehaviour, IPointerDownHandler, IPointerUpHandler,IBeginDragHandler, IEndDragHandler, IDragHandler
{

    private Vector2Int startPos = Vector2Int.zero;
    public void OnPointerDown(PointerEventData eventData)
    {
        Vector2Int fixedPos = GetDeviceFixedPos(CurrMousePosition());
        try
        {
            QScrcpy.Instance.client.Send(AssemblyControlCmd.Ass_Touch(0, AndroidMotioneventAction.AMOTION_EVENT_ACTION_DOWN, (short)fixedPos.x, (short)fixedPos.y, (short)QScrcpy.Instance.Width, (short)QScrcpy.Instance.Height));
        }
        catch (System.Exception ex)
        {
            System.Console.WriteLine(ex);
        }
        return;

        try
        {
            QScrcpy.Instance.client.Send(AssemblyControlCmd.Ass_Mouse(AndroidMotioneventAction.AMOTION_EVENT_ACTION_DOWN, AndroidMotioneventButtons.AMOTION_EVENT_BUTTON_PRIMARY, (short)fixedPos.x, (short)fixedPos.y, (short)QScrcpy.Instance.Width, (short)QScrcpy.Instance.Height));
        }
        catch (System.Exception ex)
        {
            System.Console.WriteLine(ex);
        }
    }
    public void OnPointerUp(PointerEventData eventData)
    {

        return;
        Vector2Int fixedPos = GetDeviceFixedPos(CurrMousePosition());
        try
        {
            QScrcpy.Instance.client.Send(AssemblyControlCmd.Ass_Mouse(AndroidMotioneventAction.AMOTION_EVENT_ACTION_UP, AndroidMotioneventButtons.AMOTION_EVENT_BUTTON_PRIMARY, (short)fixedPos.x, (short)fixedPos.y, (short)QScrcpy.Instance.Width, (short)QScrcpy.Instance.Height));
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

    private void Update()
    {
        if(Input.GetKeyDown(KeyCode.A))
        {
            try
            {
                QScrcpy.Instance.client.Send(AssemblyControlCmd.Ass_KeyCode(AndroidKeyeventAction.AKEY_EVENT_ACTION_DOWN, AndroidKeycode.AKEYCODE_A, AndroidMetastate.AMETA_NONE));
            }
            catch (System.Exception ex)
            {
                System.Console.WriteLine(ex);
            }
        }
        if (Input.GetKeyDown(KeyCode.B))
        {
            try
            {
                QScrcpy.Instance.client.Send(AssemblyControlCmd.Ass_Text("hello_unity")); 
            }
            catch (System.Exception ex)
            {
                System.Console.WriteLine(ex);
            }
        }

        float v = Input.GetAxis("Mouse ScrollWheel");
        if (v != 0)
        {
            Debug.Log(v * 10);
            try
            {
                QScrcpy.Instance.client.Send(AssemblyControlCmd.Ass_Scroll(
                    (short)startPos.x,
                    (short)startPos.y,
                    (short)QScrcpy.Instance.Width,
                    (short)QScrcpy.Instance.Height,
                    0,
                    (int)(v * 10)
                    ));
            }
            catch (System.Exception ex)
            {
                System.Console.WriteLine(ex);
            }
        }
    }

    public void OnBeginDrag(PointerEventData eventData)
    {
        Debug.Log("OnBeginDrag");
        startPos = GetDeviceFixedPos(CurrMousePosition());
    }

    public void OnEndDrag(PointerEventData eventData)
    {
        Debug.Log("OnEndDrag");
        Vector2Int deltaPos = GetDeviceFixedPos(CurrMousePosition()) - startPos;
        Debug.Log(deltaPos);
        try
        {
            QScrcpy.Instance.client.Send(AssemblyControlCmd.Ass_Scroll(
                (short)startPos.x,
                (short)startPos.y,
                (short)QScrcpy.Instance.Width,
                (short)QScrcpy.Instance.Height,
                deltaPos.x,
                -1
                ));
        }
        catch (System.Exception ex)
        {
            System.Console.WriteLine(ex);
        }
        //if (startPos != Vector2Int.zero) return;
        startPos = Vector2Int.zero;
    }

    public void OnDrag(PointerEventData eventData)
    {
        //Debug.Log("OnDrag");
    }

    
}
