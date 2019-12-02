using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.EventSystems;

public class DeviceController : MonoBehaviour, IPointerDownHandler, IPointerUpHandler, IDragHandler
{

    private Array keyCodeArr;
    private void Start()
    {
        keyCodeArr = Enum.GetValues(typeof(KeyCode));
    }
    public void OnPointerDown(PointerEventData eventData)
    {
        Vector2Int fixedPos = GetDeviceFixedPos(CurrMousePosition());
        Scrcpy.Instance.SendControlCmd(AssemblyControlCmd.Ass_Mouse(
                AndroidMotioneventAction.AMOTION_EVENT_ACTION_DOWN,
                AndroidMotioneventButtons.AMOTION_EVENT_BUTTON_PRIMARY,
                (short)fixedPos.x,
                (short)fixedPos.y,
                (short)Scrcpy.Instance.Width,
                (short)Scrcpy.Instance.Height));
    }
    public void OnPointerUp(PointerEventData eventData)
    {
        //if (!isPress) return;
        //isPress = false;
        Vector2Int fixedPos = GetDeviceFixedPos(CurrMousePosition());
        Scrcpy.Instance.SendControlCmd(AssemblyControlCmd.Ass_Mouse(
              AndroidMotioneventAction.AMOTION_EVENT_ACTION_UP,
              AndroidMotioneventButtons.AMOTION_EVENT_BUTTON_PRIMARY,
              (short)fixedPos.x,
              (short)fixedPos.y,
              (short)Scrcpy.Instance.Width,
              (short)Scrcpy.Instance.Height));
    }

    //获取相对于手机的屏幕坐标的位置
    private Vector2Int GetDeviceFixedPos(Vector2 mousePos)
    {
        Vector2 currentPoint = CurrMousePosition();
        Vector2 canvasSize = (transform as RectTransform).sizeDelta;
        Vector2 imgPos = new Vector2(currentPoint.x + canvasSize.x / 2, currentPoint.y + canvasSize.y / 2);
        imgPos.y = canvasSize.y - imgPos.y;
        Vector2 imgPosRate = new Vector2(imgPos.x / canvasSize.x, imgPos.y / canvasSize.y);
        Vector2Int finalyFixedPos = new Vector2Int((int)(imgPosRate.x * Scrcpy.Instance.Width), (int)(imgPosRate.y * Scrcpy.Instance.Height));
        return finalyFixedPos;
    }

    /// <summary>
    /// 获取鼠标相对于transform的位置
    /// </summary>
    /// <returns></returns>
    private Vector2 CurrMousePosition()
    {
        Vector2 vecMouse;
        RectTransformUtility.ScreenPointToLocalPointInRectangle(transform as RectTransform, Input.mousePosition, null, out vecMouse);
        return vecMouse;
    }

    private void Update()
    {
        MouseScrollEvent();
        InputEvent();
    }
    private void InputEvent()
    {
        if (Input.anyKeyDown)
        {
            foreach (KeyCode keyCode in keyCodeArr)
            {
                if (Input.GetKeyDown(keyCode))
                {
                    ConvertKeyCode((int)keyCode);
                }
            }
        }
    }
    private void ConvertKeyCode(int keycode)
    {
        //A-Z AndroidKeycode.AKEYCODE_A = 29
        if (keycode >= 97 && keycode <= 122)
        {
            KeyDown((AndroidKeycode)(keycode - 68));
        }
        //主键盘上的0-9 AKEYCODE_0 = 7
        if (keycode >= 48 && keycode <= 57)
        {
            KeyDown((AndroidKeycode)(keycode - 41));
        }

        //小键盘上的0-9 AKEYCODE_0 = 7
        if (keycode >= 256 && keycode <= 265)
        {
            KeyDown((AndroidKeycode)(keycode - 249));
        }
    }

    private void MouseScrollEvent()
    {
        float scrollValue = Input.GetAxis("Mouse ScrollWheel");
        if (scrollValue != 0)
        {
            Vector2Int currentPos = GetDeviceFixedPos(CurrMousePosition());
            Scrcpy.Instance.SendControlCmd(AssemblyControlCmd.Ass_Scroll(
               (short)currentPos.x,
               (short)currentPos.y,
               (short)Scrcpy.Instance.Width,
               (short)Scrcpy.Instance.Height,
               0,
               scrollValue
               ));
        }
    }


    public void OnDrag(PointerEventData eventData)
    {
        Vector2Int fixedPos = GetDeviceFixedPos(CurrMousePosition());
        Scrcpy.Instance.SendControlCmd(AssemblyControlCmd.Ass_Mouse(
              AndroidMotioneventAction.AMOTION_EVENT_ACTION_MOVE,
              AndroidMotioneventButtons.AMOTION_EVENT_BUTTON_PRIMARY,
              (short)fixedPos.x,
              (short)fixedPos.y,
              (short)Scrcpy.Instance.Width,
              (short)Scrcpy.Instance.Height));
    }

    public void Power()
    {
        KeyDown(AndroidKeycode.AKEYCODE_POWER);
    }

    public void SwitchAPP()
    {
        KeyDown(AndroidKeycode.AKEYCODE_APP_SWITCH);
    }
    public void Home()
    {
        KeyDown(AndroidKeycode.AKEYCODE_HOME);
    }
    public void Back()
    {
        KeyDown(AndroidKeycode.AKEYCODE_BACK);
    }

    public void VolumUp()
    {
        KeyDown(AndroidKeycode.AKEYCODE_VOLUME_UP);
    }

    public void VolumDown()
    {
        KeyDown(AndroidKeycode.AKEYCODE_VOLUME_DOWN);
    }

    private void KeyDown(AndroidKeycode keycode)
    {
        Scrcpy.Instance.SendControlCmd(AssemblyControlCmd.Ass_KeyCode(
           AndroidKeyeventAction.AKEY_EVENT_ACTION_DOWN,
           keycode,
           AndroidMetastate.AMETA_NONE));

        Scrcpy.Instance.SendControlCmd(AssemblyControlCmd.Ass_KeyCode(
           AndroidKeyeventAction.AKEY_EVENT_ACTION_UP,
           keycode,
           AndroidMetastate.AMETA_NONE));
    }
}
