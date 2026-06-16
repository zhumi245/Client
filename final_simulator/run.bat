@echo off
REM ============================================================
REM  final_simulator - 汽车信号模拟器启动脚本 (P6)
REM ============================================================
REM  启动顺序：先启动模拟器，再启动 dashboard
REM
REM  方式1（推荐）：双击本脚本，模拟器在前台控制台运行，
REM                关闭控制台即停止。
REM  方式2：命令行
REM      final_simulator.exe            （默认端口 5000）
REM      final_simulator.exe --port 5001
REM
REM  启动后，在 dashboard 目录运行：
REM      dashboard.exe --connect
REM  即可联调。
REM ============================================================

set PATH=D:\QT\Qt5.3.2\5.3\mingw482_32\bin;D:\QT\Qt5.3.2\Tools\mingw482_32\bin;%PATH%

echo.
echo  ==========================================================
echo    final_simulator  -  汽车信号模拟器 (TCP Server :5000)
echo  ==========================================================
echo   [1] 正在启动模拟器...
echo   [2] 模拟器运行后，打开另一个窗口运行 dashboard:
echo         cd ..\dashboard
echo         dashboard.exe --connect
echo   [3] 关闭本窗口可停止模拟器。
echo  ==========================================================
echo.

"%~dp0release\final_simulator.exe" %*
