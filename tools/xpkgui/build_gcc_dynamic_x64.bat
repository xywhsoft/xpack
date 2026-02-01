@echo off
echo ========================================
echo Building xpkgui x64 (GCC, dynamic)
echo ========================================
echo.

set OUTPUT_DIR=..\..\release\x64
set OUTPUT=%OUTPUT_DIR%\xpkgui.exe
set DLL_PATH=%OUTPUT_DIR%\xpack.dll
set RES=%OUTPUT_DIR%\xpkgui.res

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

if not exist "%DLL_PATH%" (
    echo Error: xpack.dll not found at %DLL_PATH%!
    echo Please run build_GCC_DLL_x64.bat first to create the DLL.
    exit /b 1
)

echo Compiling resource file...
windres -O coff -i xpkgui.rc -o "%RES%"
if errorlevel 1 (
    echo Error: Resource compilation failed
    exit /b 1
)

echo Compiling xpkgui.exe...
gcc ^
	-o "%OUTPUT%" ^
	-I../../lib ^
	-I../../src ^
	-I"%OUTPUT_DIR%" ^
	-DXPK_BUILD_DLL ^
	-DZSTD_NO_INTRINSICS ^
	-DZ7_ST ^
	-m64 ^
	-O2 ^
	-Wall ^
	xpkgui.c ^
	"%RES%" ^
	-L"%OUTPUT_DIR%" ^
	-lxpack ^
	-lcomctl32 ^
	-lcomdlg32 ^
	-lshell32 ^
	-lshlwapi ^
	-lkernel32 ^
	-luser32 ^
	-lgdi32 ^
	-ladvapi32 ^
	-lole32 ^
	-mwindows

if %ERRORLEVEL% EQU 0 (
	echo.
	echo ========================================
	echo Build successful (dynamic)!
	echo Output: %OUTPUT%
	echo Note: xpkgui.exe requires %DLL_PATH% to run.
	echo ========================================
) else (
	echo.
	echo Build failed!
	exit /b 1
)
