@echo off
rem This script runs the GameEngine with the correct working directory so it can find textures and models.
cd GameEngine
..\Debug\GameEngine.exe
if %ERRORLEVEL% NEQ 0 (
    echo Game exited with error code %ERRORLEVEL%
    pause
)
