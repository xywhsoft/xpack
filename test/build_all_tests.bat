@echo off
REM ============================================================
REM xPack Ver7 - 构建完整测试套件 (01-33)
REM ============================================================

cd /d "%~dp0.."

echo ============================================================
echo   xPack Ver7 - Building Complete Test Suite
echo ============================================================

echo.
echo Compiling with GCC x64...
gcc -m64 -I. -Isrc -Ilib -Ilib/zstd -Ilib/lzma -Ilib/xrt -DZ7_ST -O2 ^
    test/xpack_full_test_runner.c ^
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
    -o release/x64/xpack_full_test.exe

if errorlevel 1 (
    echo.
    echo ERROR: Failed to build complete test suite
    pause
    exit /b 1
)

echo.
echo ============================================================
echo   Build successful!
echo   Output: release\x64\xpack_full_test.exe
echo ============================================================

echo.
echo To run all tests:
echo   release\x64\xpack_full_test.exe
echo.
pause
