@echo off
setlocal

set ROOT=%~dp0..\..\..
set PYOCD=D:\env-windows\.venv\Scripts\pyocd.exe
set PACK=%ROOT%\tools\keil-packs\Renesas.RA_DFP.3.5.0.pack
set HEX=%ROOT%\mcu\ra6m3-hmi\Objects\rtthread.hex

"%PYOCD%" flash --pack "%PACK%" --target=r7fa6m3ah --erase=auto --frequency=500000 "%HEX%"

endlocal
