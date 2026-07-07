@echo off
setlocal

set ROOT=%~dp0..
set UV4=C:\Keil_v5\UV4\UV4.exe
set PROJECT=%ROOT%\mcu\ra6m3-hmi\project.uvprojx
set TARGET=Target 1
set LOG=%ROOT%\mcu\ra6m3-hmi\keil_build_check.log
set HEX=%ROOT%\mcu\ra6m3-hmi\Objects\rtthread.hex
set PYOCD=D:\RT-ThreadStudio\repo\Extract\Debugger_Support_Packages\RealThread\PyOCD\0.2.3\pyocd.exe
set PACK=%ROOT%\tools\keil-packs\Renesas.RA_DFP.3.5.0.pack
set PROBE_UID=0F618FD905A6
set TARGET_MCU=r7fa6m3ah3cfb

echo [1/3] Build HMI project...
"%UV4%" -j0 -b "%PROJECT%" -t "%TARGET%" -o "%LOG%"
if errorlevel 1 (
    echo Build command failed.
    type "%LOG%"
    pause
    exit /b 1
)

findstr /C:"0 Error(s)" "%LOG%" >nul
if errorlevel 1 (
    echo Build has errors:
    type "%LOG%"
    pause
    exit /b 1
)

echo [2/3] Stop old pyOCD process...
powershell -NoProfile -ExecutionPolicy Bypass -Command "Get-Process pyocd -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue"

echo [3/3] Flash firmware...
"%PYOCD%" flash --uid %PROBE_UID% --pack "%PACK%" --target=%TARGET_MCU% --erase=auto --frequency=1000000 --connect=under-reset "%HEX%"
if errorlevel 1 (
    echo Flash failed.
    pause
    exit /b 1
)

echo.
echo Download complete.
pause
