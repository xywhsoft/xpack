@echo off
echo ========================================
echo Building xpkcon x86 (TCC, dynamic)
echo ========================================
echo.

set OUTPUT_DIR=..\..\release\x86
set OUTPUT=%OUTPUT_DIR%\xpkcon.exe
set DLL_PATH=%OUTPUT_DIR%\xpack.dll

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

if not exist "%DLL_PATH%" (
    echo Error: xpack.dll not found at %DLL_PATH%!
    echo Please run build_TCC_DLL_x86.bat first to create the DLL.
    exit /b 1
)

echo Compiling xpkcon.exe...
tcc -o "%OUTPUT%" ^
	-I../../lib ^
	-I../../src ^
	-I"%OUTPUT_DIR%" ^
	-DXPK_BUILD_DLL ^
	-DZSTD_NO_INTRINSICS ^
	-DZ7_ST ^
	-m32 ^
	xpkcon.c ^
	-L"%OUTPUT_DIR%" ^
	-lxpack

if %ERRORLEVEL% EQU 0 (
	echo.
	echo ========================================
	echo Build successful (dynamic)!
	echo Output: %OUTPUT%
	echo Note: xpkcon.exe requires %DLL_PATH% to run.
	echo ========================================
) else (
	echo.
	echo Build failed!
	exit /b 1
)
