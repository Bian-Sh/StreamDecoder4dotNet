using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class SButton : MonoBehaviour {

	private enum BtnType
    {
        DeviceListBtn,
    }
    [SerializeField]
    private BtnType btnType; 
    public void OnBtnClicked()
    {
        if(btnType == BtnType.DeviceListBtn)
        {
            QScrcpy.Instance.SetSelectDevice(GetComponentInChildren<Text>().text);
        }
    }
}
