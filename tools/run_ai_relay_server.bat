@echo off
setlocal
cd /d "%~dp0"

if exist "%~dp0ai_relay_config.bat" call "%~dp0ai_relay_config.bat"

if "%AI_API_BASE%"=="" set AI_API_BASE=https://api.deepseek.com/chat/completions
if "%AI_MODEL%"=="" set AI_MODEL=deepseek-chat
if "%AI_RELAY_PORT%"=="" set AI_RELAY_PORT=8080

if "%AI_API_KEY%"=="" (
    set /p AI_API_KEY=Input DeepSeek API key:
)

python ai_relay_server.py
