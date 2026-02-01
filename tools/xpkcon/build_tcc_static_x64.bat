@echo off
echo ========================================
echo Building xpkcon x64 (TCC, static)
echo ========================================
echo.

set OUTPUT_DIR=..\..\release\x64
set OUTPUT=%OUTPUT_DIR%\xpkcon_static.exe

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

echo Compiling xpkcon_static.exe (this may take a while)...
tcc -o "%OUTPUT%" ^
	-I../../lib ^
	-I../../src ^
	-DXPK_BUILD_LIB ^
	-DZSTD_NO_INTRINSICS ^
	-DZ7_ST ^
	-DXPK_WITH_SQLITE ^
	-m64 ^
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
	../../lib/zstd/zstd.c ^
	../../src/xpack.c ^
	../../src/xpack_core.c ^
	../../src/xpack_index.c ^
	../../src/xpack_path.c ^
	../../src/xpack_util.c ^
	../../src/xpack_compress.c ^
	../../src/xpack_ldb.c ^
	../../src/xpack_volume.c ^
	xpkcon.c

if %ERRORLEVEL% EQU 0 (
	echo.
	echo ========================================
	echo Build successful (static)!
	echo Output: %OUTPUT%
	echo Note: xpkcon_static.exe is standalone, no DLL required.
	echo ========================================
) else (
	echo.
	echo Build failed!
	exit /b 1
)
