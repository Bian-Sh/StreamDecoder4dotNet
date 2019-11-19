using System.Collections;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Text.RegularExpressions;
using UnityEngine;
using UnityEngine.UI;

public class QScrcpy : MonoBehaviour {

    public GameObject devicesObject;
    public Transform devicesListParent;
    public InputField deviceSerialInput;
    public InputField deviceIPInput;
    public Toggle isWifi;
    public int reverserPort = 5555;
    private static QScrcpy instance;
    private bool isExit = false;
    private AdbController adbController;

    private Socket server;
    public static QScrcpy Instance
    {
        get { return instance; }
    }

    public System.Action UpdateEvent;

    private void Awake()
    {
        adbController = new AdbController();
        instance = this;
    }
    // Use this for initialization
    void Start () {
		
	}
    private void OnDestroy()
    {
        isExit = true;
    }
    // Update is called once per frame
    void Update () {
        if (UpdateEvent != null) UpdateEvent();
    }

    public void TryScrcpy()
    {
        PushQScrcpy();
    }
    public void ClearDevices()
    {
        RemoveQScrcpy();
        CloseReverseProxy();
    }

    public void GetDevices()
    {
        adbController.GetDevices((list) => {
            ClearDeviceList();
            for (int i = 0; i < list.Count; i++)
            {
                GameObject obj = Instantiate(devicesObject, devicesListParent);
                obj.GetComponentInChildren<Text>().text = list[i];
            }
        });
    }
    public void ADBKill()
    {
        adbController.AdbKillServer();
    }

    public void ClearDeviceList()
    {
        Transform[] allTrans = devicesListParent.GetComponentsInChildren<Transform>();
        for (int i = 0; i < allTrans.Length; i++)
        {
            if (allTrans[i] != devicesListParent.transform)
                Destroy(allTrans[i].gameObject);
        }
    }
    public void SetSelectDevice(string serial)
    {
        deviceSerialInput.text = serial;
        ClearDeviceList();
    }

    public void GetDeviceIP()
    {
        adbController.GetDeviceIP(deviceSerialInput.text, isWifi.isOn, (ip) => { deviceIPInput.text = ip; });
    }

    private void PushQScrcpy()
    {
        string local = Application.streamingAssetsPath + "/scrcpy-server.jar";
        adbController.PushQScrcpy(deviceSerialInput.text, local, () => {
            Debug.Log("push success");
            OpenReverseProxy();
        });
    }

    private void OpenReverseProxy()
    {
       
        adbController.OpenReverseProxy(deviceSerialInput.text, reverserPort, ()=> {
            Debug.Log("open reverse success localabstract:scrcpy tcp:" + reverserPort);
            server = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            try
            {
                server.Bind(new IPEndPoint(IPAddress.Parse("127.0.0.1"), reverserPort));
                server.Listen(1);
                server.BeginAccept(AcceptCb, null);
                Debug.Log("服务器启动成功");
                StartQScrcpyServer();
            }
           catch(System.Exception ex)
            {
                Debug.LogWarning(ex.ToString());
                ClearDevices();
            }
        });
    }
    private void RemoveQScrcpy()
    {
        adbController.RemoveQScrcpy(deviceSerialInput.text);
    }

    private void CloseReverseProxy()
    {
        adbController.CloseReverseProxy(deviceSerialInput.text);
    }

    private void StartQScrcpyServer()
    {
        adbController.StartQScrcpyServer(deviceSerialInput.text, 0, 10000000);
    }
    byte[] readBuff = new byte[1024];
    private void AcceptCb(System.IAsyncResult ar)
    {
        Socket client = server.EndAccept(ar);
        client.BeginReceive(readBuff, 0, 1024, SocketFlags.None, ReceiveCb, client);
    }
    private void ReceiveCb(System.IAsyncResult ar)
    {
        if (isExit) return;
        try
        {
            Socket client = (Socket)ar.AsyncState;
            int len = client.EndReceive(ar);
            if(len <= 0)
            {
                client.Shutdown(SocketShutdown.Both);
                client.Close();
                return;
            }
            Debug.Log("receive:" + len);
            client.BeginReceive(readBuff, 0, 1024, SocketFlags.None, ReceiveCb, client);
        }
        catch(System.Exception ex)
        {
            Debug.Log(ex);
        }
    }
}
