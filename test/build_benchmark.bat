@echo off
REM ============================================================
REM xPack Ver7 - 构建基准测试程序
REM ============================================================

cd /d "%~dp0.."

echo ============================================================
echo   xPack Ver7 - Building Benchmark Test Runner
echo ============================================================

echo.
echo Compiling with GCC x64...
gcc -m64 -I. -Isrc -Ilib -Ilib/zstd -Ilib/lzma -Ilib/xrt -DZ7_ST -O2 ^
    test/xpack_benchmark_runner.c ^
    lib/xrt/xrt.c ^
    src/xpack.c ^
    src/xpack_volume.c ^
    src/xpack_ldb.c ^
    src/xpack_util.c ^
    src/xpack_index.c ^
    src/xpack_path.c ^
    src/xpack_core.c ^
    src/xpack_compress.c ^
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
    -lws2_32 -lIPHLPAPI ^
    -o release/x64/xpack_benchmark.exe

if errorlevel 1 (
    echo.
    echo ERROR: Failed to build benchmark test runner
    pause
    exit /b 1
)

echo.
echo ============================================================
echo   Build successful!
echo   Output: release\x64\xpack_benchmark.exe
echo ============================================================

echo.
echo To run benchmark tests:
echo   release\x64\xpack_benchmark.exe
echo.
echo Usage:
echo   -r ^<count^>     Number of runs per benchmark (default: 5)
echo   -o ^<format^>    Output format: 0=console, 1=csv, 2=json
echo   -f ^<filename^>   Output file prefix (default: benchmark_report)
echo   -h             Show help
echo.
echo Examples:
echo   release\x64\xpack_benchmark.exe -r 10 -o 1
echo   release\x64\xpack_benchmark.exe -r 3 -o 2 -f my_report
echo.
pause
