@echo off
cd /d "%~dp0"
if exist build\Release\signal-null.exe (
    build\Release\signal-null.exe %*
    exit /b
)
if exist build\signal-null.exe (
    build\signal-null.exe %*
    exit /b
)
echo Run build.bat first. See README.md for SDL3 setup.
pause
exit /b 1
