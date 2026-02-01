@echo off
echo ========================================
echo Building xpkshext.dll x64 (GCC)
echo ========================================
echo.

set OUTPUT_DIR=..\..\release\x64
set OUTPUT=%OUTPUT_DIR%\xpkshext.dll

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

echo Compiling xpkshext.dll...
gcc -o "%OUTPUT%" ^
	-I../../lib/xrt ^
	-I../../src ^
	-DZSTD_NO_INTRINSICS ^
	-DZ7_ST ^
	-m64 ^
	-D_USRDLL ^
	-D_WINDLL ^
	-DWIN32 ^
	-D_WINDOWS ^
	-O2 ^
	-Wall ^
	-shared ^
	-Wl,--kill-at ^
	-lshell32 ^
	-lshlwapi ^
	-lole32 ^
	-lkernel32 ^
	-luser32 ^
	-ladvapi32 ^
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
