@echo off

call "%~dp0build_VC_ENV_x64.bat" || exit /b 1

where clang-cl >nul 2>nul
if errorlevel 1 (
	echo clang-cl.exe not found
	exit /b 1
)

set XPACK_CLANG_CL=clang-cl
exit /b 0
