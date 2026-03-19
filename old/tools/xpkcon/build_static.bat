@echo off
echo Building xpkcon x64 (static)...
echo.

set OUTPUT_DIR=..\..\release\x64
set OUTPUT=%OUTPUT_DIR%\xpkcon.exe

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

tcc -o "%OUTPUT%" ^
	-I../../lib -I../../src ^
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
	xpkcon.c ^
	-m64 ^
	-DZSTD_NO_INTRINSICS ^
	-DZ7_ST ^
	-DXPK_BUILD_LIB ^
	-lkernel32 ^
	-luser32 ^
	-ladvapi32

if %ERRORLEVEL% EQU 0 (
	echo.
	echo Build successful!
	echo Output: %OUTPUT%
) else (
	echo.
	echo Build failed!
)
