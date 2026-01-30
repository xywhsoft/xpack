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

REM 所有测试源文件
set TEST_SOURCES=
for %%f in (%TEST_DIR%\test_*.c) do (
    if not "%%f"=="test_runner.c" if not "%%f"=="test_framework.c" (
        set TEST_SOURCES=!TEST_SOURCES! %%f
    )
)

REM 测试文件列表
set TEST_FILES=
set TEST_FILES=!TEST_FILES! 01_core_basic.c
set TEST_FILES=!TEST_FILES! 02_core_operations.c
set TEST_FILES=!TEST_FILES! 03_core_edge_cases.c
set TEST_FILES=!TEST_FILES! 04_index_operations.c
set TEST_FILES=!TEST_FILES! 05_path_operations.c
set TEST_FILES=!TEST_FILES! 06_path_case_sensitivity.c
set TEST_FILES=!TEST_FILES! 07_compression_data_patterns.c
set TEST_FILES=!TEST_FILES! 08_compression_accuracy.c
set TEST_FILES=!TEST_FILES! 09_compression_large_files.c
set TEST_FILES=!TEST_FILES! 10_solid_compression.c
set TEST_FILES=!TEST_FILES! 11_error_handling.c
set TEST_FILES=!TEST_FILES! 12_batch_operations.c
set TEST_FILES=!TEST_FILES! 13_traverse_operations.c
set TEST_FILES=!TEST_FILES! 14_statistics.c
set TEST_FILES=!TEST_FILES! 15_verify_operations.c
set TEST_FILES=!TEST_FILES! 16_rebuild_operations.c
set TEST_FILES=!TEST_FILES! 17_package_properties.c
set TEST_FILES=!TEST_FILES! 18_file_type.c
set TEST_FILES=!TEST_FILES! 19_save_load_cycles.c
set TEST_FILES=!TEST_FILES! 20_multiple_packages.c
set TEST_FILES=!TEST_FILES! 21_edge_large_files.c
set TEST_FILES=!TEST_FILES! 22_edge_many_files.c
set TEST_FILES=!TEST_FILES! 23_concurrent_access.c
set TEST_FILES=!TEST_FILES! 24_corruption_recovery.c
set TEST_FILES=!TEST_FILES! 25_cross_platform.c
set TEST_FILES=!TEST_FILES! 26_memory_management.c
set TEST_FILES=!TEST_FILES! 27_integration_real_world.c
set TEST_FILES=!TEST_FILES! 28_performance_benchmark.c
set TEST_FILES=!TEST_FILES! 29_compression_ratio.c
set TEST_FILES=!TEST_FILES! 30_regression_tests.c

echo ============================================================
echo   xPack Ver7 - Test Compilation and Execution
echo ============================================================
echo.

REM 创建输出目录
if not exist "%OUTPUT_DIR%" (
    mkdir "%OUTPUT_DIR%"
)

REM 编译测试框架
echo [1/2] Compiling test framework...
gcc %CFLAGS% %OPTFLAGS% -c test_framework.c -o %OUTPUT_DIR%\test_framework.o
if %ERRORLEVEL% neq 0 (
    echo ERROR: Failed to compile test framework
    exit /b 1
)

REM 编译测试运行器
echo [2/2] Compiling test runner...
gcc %CFLAGS% %OPTFLAGS% -c test_runner.c -o %OUTPUT_DIR%\test_runner.o
if %ERRORLEVEL% neq 0 (
    echo ERROR: Failed to compile test runner
    exit /b 1
)

echo.
echo ============================================================
echo   Building individual test executables...
echo ============================================================
echo.

REM 清理旧的测试文件
if exist "%OUTPUT_DIR%\test_*.xpk" (
    del /Q "%OUTPUT_DIR%\test_*.xpk" 2>nul
)

REM 编译所有测试文件
set TOTAL_PASSED=0
set TOTAL_FAILED=0
set TOTAL_TIME=0

REM 遍历所有测试文件
for %%f in (%TEST_FILES%) do (
    if exist "%%f" (
        echo Compiling %%f...
        gcc %CFLAGS% %OPTFLAGS% -c %%f -o %OUTPUT_DIR%\%%~nf.o
        if %ERRORLEVEL% neq 0 (
            echo   FAILED to compile %%f
            set /a TOTAL_FAILED=TOTAL_FAILED+1
        ) else (
            echo   Linking %%f...
            gcc %OUTPUT_DIR%\%%~nf.o %OUTPUT_DIR%\test_framework.o %XPACK_SRC% %XRT_SRC% %LZ4_SRC% %ZSTD_SRC% %LZMA_SRC% %LDFLAGS% %OPTFLAGS% -o %OUTPUT_DIR%\%%~nf.exe
            if %ERRORLEVEL% neq 0 (
                echo   FAILED to link %%f
                set /a TOTAL_FAILED=TOTAL_FAILED+1
            ) else (
                echo   Running %%f...
                cd %OUTPUT_DIR%
                %%~nf.exe >%%~nf.log 2>&1
                if %ERRORLEVEL% equ 0 (
                    echo   PASSED
                    set /a TOTAL_PASSED=TOTAL_PASSED+1
                ) else (
                    echo   FAILED
                    set /a TOTAL_FAILED=TOTAL_FAILED+1
                )
                cd ..\test
            )
        )
    )
)

echo.
echo ============================================================
echo   Summary Report
echo ============================================================
echo.
echo   Test Suites Compiled and Executed
echo   -----------------------------
echo   Passed: %TOTAL_PASSED%
echo   Failed: %TOTAL_FAILED%
echo.

if %TOTAL_FAILED% equ 0 (
    echo   All tests passed!
    echo.
    exit /b 0
) else (
    echo   Some tests failed. Check log files in %OUTPUT_DIR% for details.
    echo.
    exit /b 1
)
