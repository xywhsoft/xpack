@echo off
cd /d "%~dp0"

if not exist "singlehead" mkdir "singlehead"

if exist "singlehead\single_head_maker.exe" del /f /q "singlehead\single_head_maker.exe"

gcc singlehead\single_head_maker.c -O2 -s -o singlehead\single_head_maker.exe
if errorlevel 1 exit /b %errorlevel%

singlehead\single_head_maker.exe
if errorlevel 1 exit /b %errorlevel%

echo.
echo Single header generated: singlehead\xpack.h
