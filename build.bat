@echo off
setlocal
cd /d "%~dp0"
cmake -S . -B build %*
if errorlevel 1 exit /b 1
cmake --build build --config Release
if errorlevel 1 exit /b 1
ctest --test-dir build -C Release --output-on-failure
if errorlevel 1 exit /b 1
echo Built successfully. Run play.bat
