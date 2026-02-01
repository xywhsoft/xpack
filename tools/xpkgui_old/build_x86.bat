@echo off
echo Building xpkgui x86...
echo.

windres -O coff -i xpkgui.rc -o xpkgui.res

tcc -o xpkgui.exe ^
	-I../../lib ^
	-I../../src ^
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
	xpkgui.c ^
	xpkgui.res ^
	-m32 ^
	-DZSTD_NO_INTRINSICS ^
	-DZ7_ST ^
	-DXPK_BUILD_LIB ^
	-DXPK_WITH_SQLITE ^
	comdlg32.def ^
	comctl32.def ^
	-lkernel32 ^
	-luser32 ^
	-lgdi32 ^
	-lshell32 ^
	-ladvapi32 ^
	-lole32

if %ERRORLEVEL% EQU 0 (
	echo.
	echo Build successful!
) else (
	echo.
	echo Build failed!
)

