@echo off
echo ========================================
echo Building xpkgui x86 (GCC, static)
echo ========================================
echo.

set OUTPUT_DIR=..\..\release\x86
set OUTPUT=%OUTPUT_DIR%\xpkgui_static.exe
set RES=%OUTPUT_DIR%\xpkgui.res

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

echo Compiling resource file...
windres -O coff -i xpkgui.rc -o "%RES%"
if errorlevel 1 (
    echo Error: Resource compilation failed
    exit /b 1
)

echo Compiling xpkgui_static.exe (this may take a while)...
gcc ^
	-o "%OUTPUT%" ^
	-I../../lib ^
	-I../../src ^
	-DXPK_BUILD_LIB ^
	-DZSTD_NO_INTRINSICS ^
	-DZ7_ST ^
	-DXPK_WITH_SQLITE ^
	-m32 ^
	-O2 ^
	-Wall ^
	../../lib/xrt/xrt.c ^
	../../lib/lz4/lz4.c ^
	../../lib/lz4/lz4hc.c ^
	../../lib/lzma/Alloc.c ^
	../../lib/lzma/CpuArch.c ^
	../../lib/lzma/LzFind.c ^
	../../lib/lzma/LzmaDec.c ^
	../../lib/lzma/LzmaEnc.c ^
	../../lib/lzma/Lzma2Dec.c ^
	../../lib/lzma/Lzma2Enc.c ^
	../../lib/lzma/tcc_stub.c ^
	../../lib/zstd/zstd.c ^
	../../src/xpack.c ^
	../../src/xpack_core.c ^
	../../src/xpack_index.c ^
	../../src/xpack_path.c ^
	../../src/xpack_util.c ^
	../../src/xpack_compress.c ^
	../../src/xpack_ldb.c ^
	../../src/xpack_volume.c ^
	xpkgui.c ^
	"%RES%" ^
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
	echo Build successful (static)!
	echo Output: %OUTPUT%
	echo Note: xpkgui_static.exe is standalone, no DLL required.
	echo ========================================
) else (
	echo.
	echo Build failed!
	exit /b 1
)
