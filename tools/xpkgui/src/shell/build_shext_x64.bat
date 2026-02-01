@echo off
setlocal

echo ========================================
echo Building xpkshext.dll (x64)
echo ========================================
echo.

set CC=cl
set CFLAGS=/O2 /MT /DWIN32 /D_WINDOWS /D_USRDLL /D_WINDLL /I..\..\..\include /I..\..\..\lib\xrt\include /I"%WindowsSdkDir%Include\%WindowsSDKVersion%um" /I"%WindowsSdkDir%Include\%WindowsSDKVersion%shared"
set LDFLAGS=/DLL /LIBPATH:..\..\..\lib\xrt\build_x64 /LIBPATH:"%WindowsSdkDir%Lib\%WindowsSDKVersion%um\x64"
set LIBS=shell32.lib shlwapi.lib ole32.lib xrt.lib

if not exist build_x64 mkdir build_x64

echo Compiling xpkshext.c...
%CC% %CFLAGS% /c xpkshext.c /Fobuild_x64\xpkshext.obj
if errorlevel 1 (
    echo Error: Compilation failed
    exit /b 1
)

echo Linking xpkshext.dll...
link %LDFLAGS% /DEF:xpkshext.def build_x64\xpkshext.obj %LIBS% /OUT:build_x64\xpkshext.dll
if errorlevel 1 (
    echo Error: Linking failed
    exit /b 1
)

echo.
echo ========================================
echo Build successful: build_x64\xpkshext.dll
echo ========================================
