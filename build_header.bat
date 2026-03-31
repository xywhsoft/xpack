@echo off
cd /d "%~dp0"
call build_single_head.bat
exit /b %errorlevel%
