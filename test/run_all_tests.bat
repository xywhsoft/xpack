@echo off
setlocal enabledelayedexpansion

REM ============================================================
REM xPack Ver7 - 测试运行批处理脚本
REM ============================================================

set SRC_DIR=..\src
set TEST_DIR=.
set LIB_DIR=..\lib
set OUTPUT_DIR=..\release\x64
set XRT_DIR=%LIB_DIR%\xrt

REM 编译参数
set CFLAGS=-m64 -I.. -I..\src -I%LIB_DIR% -I%LIB_DIR%\zstd -I%LIB_DIR%\lzma -DZ7_ST -DDEBUG_TRACE
set LDFLAGS=-lws2_32 -lIPHLPAPI
set OPTFLAGS=-O2 -s -ffunction-sections -fdata-sections -Wl,--gc-sections

REM 源文件列表
set XPACK_SRC=%SRC_DIR%\xpack.c %SRC_DIR%\xpack_compress.c %SRC_DIR%\xpack_core.c %SRC_DIR%\xpack_index.c %SRC_DIR%\xpack_ldb.c %SRC_DIR%\xpack_path.c %SRC_DIR%\xpack_util.c
set XRT_SRC=%XRT_DIR%\xrt.c
set LZ4_SRC=%LIB_DIR%\lz4\lz4.c %LIB_DIR%\lz4\lz4hc.c
set ZSTD_SRC=%LIB_DIR%\zstd\zstd.c
set LZMA_SRC=%LIB_DIR%\lzma\Alloc.c %LIB_DIR%\lzma\CpuArch.c %LIB_DIR%\lzma\LzFind.c %LIB_DIR%\lzma\LzmaDec.c %LIB_DIR%\lzma\LzmaEnc.c %LIB_DIR%\lzma\Lzma2Dec.c %LIB_DIR%\lzma\Lzma2Enc.c

echo ============================================================
echo   xPack Ver7 - Test Compilation and Execution
echo ============================================================
echo.

REM 创建输出目录
if not exist "%OUTPUT_DIR%" (
    mkdir "%OUTPUT_DIR%"
)

REM 清理旧的测试文件
if exist "%OUTPUT_DIR%\test_*.xpk" (
    del /Q "%OUTPUT_DIR%\test_*.xpk" 2>nul
)

REM 编译测试运行器
echo [1/2] Compiling test runner...
gcc %CFLAGS% %OPTFLAGS% test_framework.c test_main.c %XPACK_SRC% %XRT_SRC% %LZ4_SRC% %ZSTD_SRC% %LZMA_SRC% %LDFLAGS% -o %OUTPUT_DIR%\test_all.exe
if %ERRORLEVEL% neq 0 (
    echo ERROR: Failed to compile test runner
    exit /b 1
)

echo [2/2] Running test suite...
cd %OUTPUT_DIR%
test_all.exe > test_all.log 2>&1
set RESULT=%ERRORLEVEL%
cd ..\test

echo.
echo ============================================================
echo   Test Execution Complete
echo ============================================================
echo.

REM 显示日志内容
type %OUTPUT_DIR%\test_all.log

echo.
if %RESULT% equ 0 (
    echo ============================================================
    echo   All tests passed!
    echo ============================================================
    echo.
    exit /b 0
) else (
    echo ============================================================
    echo   Some tests failed. Check %OUTPUT_DIR%\test_all.log for details.
    echo ============================================================
    echo.
    exit /b 1
)
