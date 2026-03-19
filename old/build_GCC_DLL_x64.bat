


set LIB_DIR=lib
set LZ4_DIR=lib/lz4
set LZMA_DIR=lib/lzma
set ZSTD_DIR=lib/zstd



set CFLAGS=-I%LIB_DIR% -I%LZ4_DIR% -I%LZMA_DIR% -I%ZSTD_DIR% -DZ7_ST -DXPK_BUILD_DLL -O2 -s -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections
set LDFLAGS=-lws2_32 -liphlpapi



gcc -m64 -shared ^
	src/xpack.c ^
	src/xpack_compress.c ^
	src/xpack_core.c ^
	src/xpack_index.c ^
	src/xpack_ldb.c ^
	src/xpack_path.c ^
	src/xpack_util.c ^
	src/xpack_volume.c ^
	lib/xrt/xrt.c ^
	lib/lz4/lz4.c ^
	lib/lz4/lz4hc.c ^
	lib/zstd/zstd.c ^
	lib/lzma/Alloc.c ^
	lib/lzma/CpuArch.c ^
	lib/lzma/LzFind.c ^
	lib/lzma/LzmaDec.c ^
	lib/lzma/LzmaEnc.c ^
	lib/lzma/Lzma2Dec.c ^
	lib/lzma/Lzma2Enc.c ^
	%CFLAGS%  ^
	%LDFLAGS% ^
	-o release/x64/xpack.dll

@echo;
@echo off

pause
