@echo off
::设置FFMpeg路径
set dllpath=C:\Library\Adb
if not exist adb.exe (copy %dllpath%\adb.exe .\)
if not exist AdbWinApi.dll (copy %dllpath%\AdbWinApi.dll .\)
if not exist AdbWinUsbApi.dll (copy %dllpath%\AdbWinUsbApi.dll .\)
if not exist scrcpy-server.jar (copy %dllpath%\scrcpy-server.jar .\)



