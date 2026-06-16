@echo off
cd /d "%~dp0"
set "PATH=D:\QT\Qt5.3.2\5.3\mingw482_32\bin;D:\QT\Qt5.3.2\Tools\mingw482_32\bin;%PATH%"
set "QMAKESPEC=win32-g++"

REM Default: Network mode (connect to simulator at 127.0.0.1:5000)
REM For standalone mode: dashboard.exe --mock
REM Prerequisite: final_simulator must be running first

start "dashboard" "%~dp0release\dashboard.exe" %*
echo.
echo Dashboard started (Network mode - connecting to simulator...)
echo To run standalone: dashboard.exe --mock
