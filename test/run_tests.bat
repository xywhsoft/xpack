@echo off
setlocal

set SRC_DIR=..\src
set TEST_DIR=.
set LIB_DIR=..\lib
set OUTPUT_DIR=..\release\x64
set XRT_DIR=%LIB_DIR%\xrt

echo ============================================================
echo   xPack Ver7 - Test Compilation and Execution
echo ============================================================
echo.

cd /d %~dp0

if not exist "%OUTPUT_DIR%" (
    mkdir "%OUTPUT_DIR%"
)

del /Q "%OUTPUT_DIR%\test_*.xpk" 2>nul

echo [1/2] Compiling test runner...
gcc -m64 -I.. -I..\src -I%LIB_DIR% -I%LIB_DIR%\zstd -I%LIB_DIR%\lzma -DZ7_ST -DDEBUG_TRACE -O2 -s -ffunction-sections -fdata-sections -Wl,--gc-sections test_main.c %SRC_DIR%\xpack.c %SRC_DIR%\xpack_compress.c %SRC_DIR%\xpack_core.c %SRC_DIR%\xpack_index.c %SRC_DIR%\xpack_ldb.c %SRC_DIR%\xpack_path.c %SRC_DIR%\xpack_util.c %XRT_DIR%\xrt.c %LIB_DIR%\lz4\lz4.c %LIB_DIR%\lz4\lz4hc.c %LIB_DIR%\zstd\zstd.c %LIB_DIR%\lzma\Alloc.c %LIB_DIR%\lzma\CpuArch.c %LIB_DIR%\lzma\LzFind.c %LIB_DIR%\lzma\LzmaDec.c %LIB_DIR%\lzma\LzmaEnc.c %LIB_DIR%\lzma\Lzma2Dec.c %LIB_DIR%\lzma\Lzma2Enc.c -lws2_32 -lIPHLPAPI -o %OUTPUT_DIR%\test_runner.exe

if %ERRORLEVEL% neq 0 (
    echo   ERROR: Failed to compile test runner
    goto :error
)

echo [2/2] Running test suite...
cd /d %OUTPUT_DIR%
test_runner.exe
set RESULT=%ERRORLEVEL%
cd /d %TEST_DIR%

echo.
if %RESULT% equ 0 (
    echo ============================================================
    echo   All tests passed!
    echo ============================================================
    echo.
    exit /b 0
) else (
    echo ============================================================
    echo   Some tests failed. See output above for details.
    echo ============================================================
    echo.
    exit /b 1
)

:error
echo.
echo ============================================================
echo   Build failed. Check compilation errors above.
echo ============================================================
echo.
exit /b 1
