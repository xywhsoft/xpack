@echo off
echo ========================================
echo Building xpkshext.dll x86 (TCC)
echo ========================================
echo.

set OUTPUT_DIR=..\..\release\x86
set OUTPUT=%OUTPUT_DIR%\xpkshext.dll

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

echo Compiling xpkshext.dll...
tcc -o "%OUTPUT%" ^
	-I../../lib/xrt ^
	-I../../src ^
	-DZSTD_NO_INTRINSICS ^
	-DZ7_ST ^
	-m32 ^
	-D_USRDLL ^
	-D_WINDLL ^
	-DWIN32 ^
	-D_WINDOWS ^
	-lshell32 ^
	-lshlwapi ^
	-lole32 ^
	-lkernel32 ^
	-luser32 ^
	-ladvapi32 ^
	-DLL ^
	shared ^
	xpkshext.c

if %ERRORLEVEL% EQU 0 (
	echo.
	echo ========================================
	echo Build successful!
	echo Output: %OUTPUT%
	echo Note: Use install.bat to register the shell extension.
	echo ========================================
) else (
	echo.
	echo Build failed!
	exit /b 1
)
