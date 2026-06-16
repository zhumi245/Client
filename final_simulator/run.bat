@echo off
cd /d "%~dp0"
set "PATH=D:\QT\Qt5.3.2\5.3\mingw482_32\bin;D:\QT\Qt5.3.2\Tools\mingw482_32\bin;%PATH%"
set "QMAKESPEC=win32-g++"

echo.
echo  ==========================================================
echo    final_simulator  -  汽车信号模拟器 (TCP Server :5000)
echo  ==========================================================
echo   [1] 正在启动模拟器...
echo   [2] 模拟器运行后，打开另一个窗口运行 dashboard:
echo         cd ..\work\dashboard
echo         dashboard.exe
echo   [3] 关闭本窗口可停止模拟器。
echo  ==========================================================
echo.

start "final_simulator" "%~dp0release\final_simulator.exe" %*
