@echo off
echo Building xpkcon x64 (dynamic linking with xpack.dll)...
echo.

set OUTPUT_DIR=..\..\release\x64
set OUTPUT=%OUTPUT_DIR%\xpkcon.exe
set DLL_PATH=%OUTPUT_DIR%\xpack.dll

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

if not exist "%DLL_PATH%" (
    echo Error: xpack.dll not found at %DLL_PATH%!
    echo Please run build_TCC_DLL_x64.bat first to create the DLL.
    exit /b 1
)

tcc -o "%OUTPUT%" ^
	-I../../lib ^
	-I../../src ^
	../../lib/xrt/xrt.c ^
	xpkcon.c ^
	-m64 ^
	-DXPK_BUILD_DLL ^
	-L"%OUTPUT_DIR%" ^
	-lxpack ^
	-lkernel32 ^
	-luser32 ^
	-ladvapi32

if %ERRORLEVEL% EQU 0 (
	echo.
	echo Build successful (dynamic)!
	echo Output: %OUTPUT%
	echo Note: xpkcon.exe requires %DLL_PATH% to run.
) else (
	echo.
	echo Build failed!
)
