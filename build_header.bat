@echo off
cd /d "%~dp0"

if not exist "release\x64" mkdir "release\x64"

gcc tools\make_header\make_header.c -O2 -s -o release\x64\make_header.exe
if errorlevel 1 exit /b %errorlevel%

release\x64\make_header.exe
if errorlevel 1 exit /b %errorlevel%

echo.
echo Header generated: xpack.h
