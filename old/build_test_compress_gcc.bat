@echo off
REM xPack Ver7 - 压缩模块测试编译脚本 (GCC)
REM
REM 使用方法:
REM   build_test_compress_gcc.bat

echo ================================================
echo  xPack Ver7 - Compression Test Build (GCC 15.2)
echo ================================================

REM 编译器标志
REM -DZ7_ST: 禁用 LZMA2 多线程（避免缺少 MtCoder.h）
set CFLAGS=-g -O0 -DZ7_ST
set INCLUDES=-Ilib/xrt -Ilib/lz4 -Ilib/zstd -Ilib/lzma

REM 源文件
set SRCS=test_compress.c xpack.c

REM 库源文件
set LIB_SRCS=lib/lz4/lz4.c lib/lz4/lz4hc.c lib/zstd/zstd.c lib/lzma/Alloc.c lib/lzma/Lzma2Enc.c lib/lzma/Lzma2Dec.c lib/lzma/LzmaEnc.c lib/lzma/LzmaDec.c lib/lzma/LzFind.c lib/lzma/CpuArch.c

REM 链接库（xrt 需要）
set LIBS=-lws2_32 -liphlpapi

REM 输出文件
set OUTPUT=test_compress_gcc.exe

echo.
echo [1/2] Compiling with GCC...
gcc %CFLAGS% %INCLUDES% %SRCS% %LIB_SRCS% -o %OUTPUT% %LIBS%

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
