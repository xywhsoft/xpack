@echo off
setlocal

set BIN_DIR=bin
set INC_DIR=include
set XRT_DIR=../../lib/xrt
set XPACK_DIR=../../src
set LZ4_DIR=../../lib/lz4
set LZMA_DIR=../../lib/lzma
set ZSTD_DIR=../../lib/zstd

set CFLAGS=-I%INC_DIR% -I%XRT_DIR% -I%XPACK_DIR% -I%LZ4_DIR% -I%LZMA_DIR% -I%ZSTD_DIR% -DZ7_ST -O2 -municode -DUNICODE
set LDFLAGS=-mwindows -lshell32 -lcomctl32 -lshlwapi -lws2_32 -liphlpapi -lcomdlg32

if not exist %BIN_DIR% mkdir %BIN_DIR%

echo Compiling resources...
windres -i src/resources/xpkgui.rc -o obj/xpkgui_res.o -O coff --codepage=65001

echo Compiling xpkgui...
gcc src/main.c %XRT_DIR%/xrt.c %LZ4_DIR%/lz4.c %LZ4_DIR%/lz4hc.c %LZMA_DIR%/Alloc.c %LZMA_DIR%/CpuArch.c %LZMA_DIR%/LzFind.c %LZMA_DIR%/LzmaDec.c %LZMA_DIR%/LzmaEnc.c %LZMA_DIR%/Lzma2Dec.c %LZMA_DIR%/Lzma2Enc.c %ZSTD_DIR%/zstd.c %XPACK_DIR%/xpack.c %XPACK_DIR%/xpack_core.c %XPACK_DIR%/xpack_index.c %XPACK_DIR%/xpack_path.c %XPACK_DIR%/xpack_util.c %XPACK_DIR%/xpack_compress.c %XPACK_DIR%/xpack_ldb.c %XPACK_DIR%/xpack_volume.c obj/xpkgui_res.o %CFLAGS% %LDFLAGS% -o %BIN_DIR%/xpkgui.exe

gcc src/main.c %XRT_DIR%/xrt.c %LZ4_DIR%/lz4.c %LZ4_DIR%/lz4hc.c %LZMA_DIR%/Alloc.c %LZMA_DIR%/CpuArch.c %LZMA_DIR%/LzFind.c %LZMA_DIR%/LzmaDec.c %LZMA_DIR%/LzmaEnc.c %LZMA_DIR%/Lzma2Dec.c %LZMA_DIR%/Lzma2Enc.c %ZSTD_DIR%/zstd.c %XPACK_DIR%/xpack.c %XPACK_DIR%/xpack_core.c %XPACK_DIR%/xpack_index.c %XPACK_DIR%/xpack_path.c %XPACK_DIR%/xpack_util.c %XPACK_DIR%/xpack_compress.c %XPACK_DIR%/xpack_ldb.c %XPACK_DIR%/xpack_volume.c obj/xpkgui_res.o %CFLAGS% -lshell32 -lcomctl32 -lshlwapi -lws2_32 -liphlpapi -lcomdlg32 -o %BIN_DIR%/xpkgui_debug.exe

if exist %BIN_DIR%/xpkgui.exe (
    echo Build successful: %BIN_DIR%/xpkgui.exe
) else (
    echo Build failed!
)
endlocal
