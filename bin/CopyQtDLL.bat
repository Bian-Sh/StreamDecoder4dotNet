@echo off
::设置QT路径
set dllpath=C:\Qt\Qt5.9.8\5.9.8\msvc2017_64
if not exist Qt5Core.dll (copy %dllpath%\bin\Qt5Core.dll .\)
if not exist Qt5Cored.dll (copy %dllpath%\bin\Qt5Cored.dll .\)
if not exist Qt5Gui.dll (copy %dllpath%\bin\Qt5Gui.dll .\)
if not exist Qt5Guid.dll (copy %dllpath%\bin\Qt5Guid.dll .\)
if not exist Qt5Multimedia.dll (copy %dllpath%\bin\Qt5Multimedia.dll .\)
if not exist Qt5Multimediad.dll (copy %dllpath%\bin\Qt5Multimediad.dll .\)
if not exist Qt5Network.dll (copy %dllpath%\bin\Qt5Network.dll .\)
if not exist Qt5Networkd.dll (copy %dllpath%\bin\Qt5Networkd.dll .\)
if not exist Qt5OpenGL.dll (copy %dllpath%\bin\Qt5OpenGL.dll .\)
if not exist Qt5OpenGLd.dll (copy %dllpath%\bin\Qt5OpenGLd.dll .\)
if not exist Qt5Widgets.dll (copy %dllpath%\bin\Qt5Widgets.dll .\)
if not exist Qt5Widgetsd.dll (copy %dllpath%\bin\Qt5Widgetsd.dll .\)

::创建目录
if not exist platforms (mkdir platforms)
if not exist platforms/qwindows.dll (copy %dllpath%\plugins\platforms\qwindows.dll .\platforms)
if not exist platforms/qwindowsd.dll (copy %dllpath%\plugins\platforms\qwindowsd.dll .\platforms)


if not exist audio (mkdir audio)
if not exist audio/qtaudio_wasapi.dll (copy %dllpath%\plugins\audio\qtaudio_wasapi.dll .\audio)
if not exist audio/qtaudio_wasapid.dll (copy %dllpath%\plugins\audio\qtaudio_wasapid.dll .\audio)
if not exist audio/qtaudio_windows.dll (copy %dllpath%\plugins\audio\qtaudio_windows.dll .\audio)
if not exist audio/qtaudio_windowsd.dll (copy %dllpath%\plugins\audio\qtaudio_windowsd.dll .\audio)

