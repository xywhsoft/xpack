@echo off
echo Building xpkcon x86...
echo.

set OUTPUT_DIR=..\..\release\x86
set OUTPUT=%OUTPUT_DIR%\xpkcon.exe
set DLL_PATH=%OUTPUT_DIR%\xpack.dll

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

if exist "%DLL_PATH%" (
	echo Using existing xpack.dll (recommended)
	echo.

	tcc -o "%OUTPUT%" ^
		-I../../lib ^
		-I../../src ^
		../../lib/xrt/xrt.c ^
		xpkcon.c ^
		-m32 ^
		-DXPK_BUILD_DLL ^
		-L"%OUTPUT_DIR%" ^
		-lxpack ^
		-lkernel32 ^
		-luser32 ^
		-ladvapi32

	if %ERRORLEVEL% EQU 0 (
		echo.
		echo Build successful with DLL!
		echo Output: %OUTPUT%
		echo Note: xpkcon.exe requires %DLL_PATH% to run.
		goto :end
	)
)

echo.
echo Static linking (no DLL needed)...
echo.

tcc -o "%OUTPUT%" ^
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
	-m32 ^
	-DZSTD_NO_INTRINSICS ^
	-DZ7_ST ^
	-DXPK_BUILD_LIB ^
	-lkernel32 ^
	-luser32 ^
	-ladvapi32

if %ERRORLEVEL% EQU 0 (
	echo.
	echo Build successful (static)!
	echo Output: %OUTPUT%
) else (
	echo.
	echo Build failed!
)

:end
