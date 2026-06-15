@echo off
set PATH=D:\QT\Qt5.3.2\5.3\mingw482_32\bin;D:\QT\Qt5.3.2\Tools\mingw482_32\bin;%PATH%

REM Default: Mock mode (standalone, no simulator needed)
REM For network mode: dashboard.exe --connect

start "" "%~dp0release\dashboard.exe"
echo.
echo Dashboard started in Mock mode.
echo To connect to simulator, run: dashboard.exe --connect
