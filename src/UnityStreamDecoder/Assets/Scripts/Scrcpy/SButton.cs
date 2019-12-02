using UnityEngine;
using UnityEngine.UI;

public class SButton : MonoBehaviour {

    public void OnBtnClicked()
    {
        Scrcpy.Instance.SetSelectDevice(GetComponentInChildren<Text>().text);
    }
}
