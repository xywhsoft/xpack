@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
set "ROOT_DIR=%SCRIPT_DIR%..\.."
for %%I in ("%ROOT_DIR%") do set "ROOT_DIR=%%~fI"
set "PROJ_DIR=%ROOT_DIR%\tools\xpkgui"
set "SRC_DIR=%PROJ_DIR%\src"
set "OBJ_DIR=%PROJ_DIR%\obj"
set "REL_DIR=%ROOT_DIR%\release\x64"

call :ensure_env || exit /b 1

if not exist "%OBJ_DIR%" mkdir "%OBJ_DIR%" || exit /b 1
if not exist "%REL_DIR%" mkdir "%REL_DIR%" || exit /b 1

del /f /q "%OBJ_DIR%\*.obj" "%OBJ_DIR%\*.res" "%OBJ_DIR%\*.ilk" "%OBJ_DIR%\*.pdb" "%OBJ_DIR%\xpkgui_shext.lib" "%OBJ_DIR%\xpkgui_shext.exp" >nul 2>nul
del /f /q "%REL_DIR%\xpkgui.exe" "%REL_DIR%\xpkgui_shext.dll" "%REL_DIR%\xpkgui_shext.lib" "%REL_DIR%\xpkgui_shext.exp" >nul 2>nul

set "COMMON_DEFS=/DUNICODE /D_UNICODE /DWIN32_LEAN_AND_MEAN /DZ7_ST /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS /D_WINSOCK_DEPRECATED_NO_WARNINGS /DXRT_NO_COROUTINE /DXRT_NO_NETWORK /DXRT_NO_XURL /DXRT_NO_HTTP_UTIL /DXRT_NO_XCODEC /DXRT_NO_CRYPTO /DXRT_NO_NETTLS /DXRT_NO_XHTTP /DXRT_NO_XHTTPD /DXRT_NO_XWS /DXRT_NO_XID /DXRT_NO_BUFFER /DXRT_NO_STACK /DXRT_NO_REGEX /DXRT_NO_VALUE /DXRT_NO_JSON /DXRT_NO_TEMPLATE"
set "COMMON_INC=/I%PROJ_DIR%\compat /I%ROOT_DIR% /I%SRC_DIR% /I%ROOT_DIR%\lib\lz4 /I%ROOT_DIR%\lib\zstd /I%ROOT_DIR%\lib\lzma"
set "COMMON_CFLAGS=/nologo /TC /utf-8 /std:c11 /O2 /W0 %COMMON_DEFS% %COMMON_INC%"
set "GUI_SRCS=%SRC_DIR%\main.c %SRC_DIR%\common.c %SRC_DIR%\dialogs.c %SRC_DIR%\archive.c %SRC_DIR%\window.c %ROOT_DIR%\xpack.c %ROOT_DIR%\lib\lz4\lz4.c %ROOT_DIR%\lib\lz4\lz4hc.c %ROOT_DIR%\lib\zstd\zstd.c %ROOT_DIR%\lib\lzma\Alloc.c %ROOT_DIR%\lib\lzma\CpuArch.c %ROOT_DIR%\lib\lzma\LzFind.c %ROOT_DIR%\lib\lzma\LzmaDec.c %ROOT_DIR%\lib\lzma\LzmaEnc.c %ROOT_DIR%\lib\lzma\Lzma2Dec.c %ROOT_DIR%\lib\lzma\Lzma2Enc.c"
set "GUI_OBJS=%OBJ_DIR%\main.obj %OBJ_DIR%\common.obj %OBJ_DIR%\dialogs.obj %OBJ_DIR%\archive.obj %OBJ_DIR%\window.obj %OBJ_DIR%\xpack.obj %OBJ_DIR%\lz4.obj %OBJ_DIR%\lz4hc.obj %OBJ_DIR%\zstd.obj %OBJ_DIR%\Alloc.obj %OBJ_DIR%\CpuArch.obj %OBJ_DIR%\LzFind.obj %OBJ_DIR%\LzmaDec.obj %OBJ_DIR%\LzmaEnc.obj %OBJ_DIR%\Lzma2Dec.obj %OBJ_DIR%\Lzma2Enc.obj"
set "GUI_LIBS=User32.lib Kernel32.lib Gdi32.lib Comctl32.lib Comdlg32.lib Shell32.lib Shlwapi.lib Ole32.lib OleAut32.lib Advapi32.lib Uuid.lib Ws2_32.lib IPHLPAPI.lib"
set "SHELL_LIBS=User32.lib Kernel32.lib Shell32.lib Shlwapi.lib Ole32.lib OleAut32.lib Advapi32.lib Uuid.lib"

echo [1/4] Compiling GUI sources...
"%CC%" %COMMON_CFLAGS% /c /Fo%OBJ_DIR%\\ %GUI_SRCS% || exit /b 1

echo [2/4] Compiling resources...
rc /nologo /fo "%OBJ_DIR%\xpkgui.res" "%SRC_DIR%\resources\xpkgui.rc" || exit /b 1

echo [3/4] Linking xpkgui.exe...
link /nologo /OUT:"%REL_DIR%\xpkgui.exe" %GUI_OBJS% "%OBJ_DIR%\xpkgui.res" %GUI_LIBS% || exit /b 1

echo [4/4] Building shell extension...
"%CC%" /nologo /TC /utf-8 /std:c11 /O2 /W3 /DUNICODE /D_UNICODE /DWIN32_LEAN_AND_MEAN /I"%SRC_DIR%" /I"%ROOT_DIR%" /c /Fo%OBJ_DIR%\\ "%SRC_DIR%\shell\xpkgui_shext.c" || exit /b 1
link /nologo /DLL /OUT:"%REL_DIR%\xpkgui_shext.dll" /IMPLIB:"%OBJ_DIR%\xpkgui_shext.lib" /DEF:"%SRC_DIR%\shell\xpkgui_shext.def" "%OBJ_DIR%\xpkgui_shext.obj" %SHELL_LIBS% || exit /b 1

echo.
echo Build successful:
echo   %REL_DIR%\xpkgui.exe
echo   %REL_DIR%\xpkgui_shext.dll
exit /b 0

:ensure_env
where /q clang-cl
if not errorlevel 1 (
	set "CC=clang-cl"
	where /q rc
	if errorlevel 1 goto :load_vs_env
	goto :eof
)

:load_vs_env
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
	echo [ERROR] clang-cl/rc not found, and vswhere.exe is unavailable.
	exit /b 1
)

for /f "usebackq delims=" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSINSTALL=%%I"
if not defined VSINSTALL (
	echo [ERROR] No Visual Studio C/C++ toolchain was found.
	exit /b 1
)

call "%VSINSTALL%\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul || exit /b 1

where /q clang-cl
if errorlevel 1 (
	if exist "%VSINSTALL%\VC\Tools\Llvm\x64\bin\clang-cl.exe" (
		set "CC=%VSINSTALL%\VC\Tools\Llvm\x64\bin\clang-cl.exe"
	) else if exist "C:\Program Files\LLVM\bin\clang-cl.exe" (
		set "CC=C:\Program Files\LLVM\bin\clang-cl.exe"
	) else (
		echo [ERROR] clang-cl not found. Install LLVM support for Visual Studio or standalone LLVM.
		exit /b 1
	)
) else (
	set "CC=clang-cl"
)

where /q rc
if errorlevel 1 (
	echo [ERROR] rc.exe not found after loading the Visual Studio environment.
	exit /b 1
)
exit /b 0
