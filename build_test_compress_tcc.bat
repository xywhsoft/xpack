@echo off
REM xPack Ver7 - 压缩模块测试编译脚本 (TCC)
REM
REM 使用方法:
REM   build_test_compress_tcc.bat

echo ================================================
echo  xPack Ver7 - Compression Test Build (TCC 0.9.27)
echo ================================================

REM 编译器标志
REM -DZ7_ST: 禁用 LZMA2 多线程（避免缺少 MtCoder.h）
REM -DZSTD_NO_INTRINSICS: 禁用 ZSTD SIMD（TCC 不支持 emmintrin.h）
set CFLAGS=-g -DZ7_ST -DZSTD_NO_INTRINSICS
set INCLUDES=-Ilib/xrt -Ilib/lz4 -Ilib/zstd -Ilib/lzma

REM 源文件
set SRCS=test_compress.c xpack.c

REM 库源文件
set LIB_SRCS=lib/lz4/lz4.c lib/lz4/lz4hc.c lib/zstd/zstd.c lib/lzma/Alloc.c lib/lzma/Lzma2Enc.c lib/lzma/Lzma2Dec.c lib/lzma/LzmaEnc.c lib/lzma/LzmaDec.c lib/lzma/LzFind.c lib/lzma/CpuArch.c

REM 链接库（xrt 需要）
set LIBS=-lws2_32 -liphlpapi

REM 输出文件
set OUTPUT=test_compress_tcc.exe

echo.
echo [1/2] Compiling with TCC...
tcc %CFLAGS% %INCLUDES% %SRCS% %LIB_SRCS% -o %OUTPUT% %LIBS%

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Compilation failed!
    exit /b 1
)

echo.
echo [2/2] Build successful: %OUTPUT%
echo.
echo To run: %OUTPUT%
echo.
