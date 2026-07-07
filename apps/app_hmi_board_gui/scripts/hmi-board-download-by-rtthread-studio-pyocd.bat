@echo off
setlocal

set PYOCD=D:\RT-ThreadStudio\repo\Extract\Debugger_Support_Packages\RealThread\PyOCD\0.2.3\pyocd.exe
set PYOCD_DIR=D:\RT-ThreadStudio\repo\Extract\Debugger_Support_Packages\RealThread\PyOCD\0.2.3
set HEX=D:\qianrushi\xiotman-master\mcu\ra6m3-hmi\Objects\rtthread.hex

cd /d "%PYOCD_DIR%"
"%PYOCD%" flash --target=R7FA6M3AH --erase=auto --frequency=1000000 "%HEX%"

pause
endlocal
