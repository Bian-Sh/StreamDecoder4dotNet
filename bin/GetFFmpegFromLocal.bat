@echo off
::设置FFMpeg路径
set dllpath=C:\Library\FFMPEG
if not exist avcodec-57.dll (copy %dllpath%\avcodec-57.dll .\)
if not exist avdevice-57.dll (copy %dllpath%\avdevice-57.dll .\)
if not exist avfilter-6.dll (copy %dllpath%\avfilter-6.dll .\)
if not exist avformat-57.dll (copy %dllpath%\avformat-57.dll .\)
if not exist avutil-55.dll (copy %dllpath%\avutil-55.dll .\)
if not exist postproc-54.dll (copy %dllpath%\postproc-54.dll .\)
if not exist swresample-2.dll (copy %dllpath%\swresample-2.dll .\)
if not exist swscale-4.dll (copy %dllpath%\swscale-4.dll .\)


