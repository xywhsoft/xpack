@echo off
setlocal enabledelayedexpansion

echo =================================================
echo   xpkcon Test Runner
echo =================================================
echo.

set OUTPUT_DIR=..\..\..\release\x64
set TEST_EXE=%OUTPUT_DIR%\test_xpkcon.exe

if not exist "%TEST_EXE%" (
    echo Test executable not found, building...
    call build_test.bat
    if !ERRORLEVEL! NEQ 0 (
        echo Build failed!
        exit /b 1
    )
    echo.
)

echo Running tests...
echo.
echo Test executable: %TEST_EXE%
echo.

"%TEST_EXE%"

set RESULT=!ERRORLEVEL!

echo.

if !RESULT! EQU 0 (
    echo All tests passed!
) else (
    echo Some tests failed!
)

exit /b !RESULT!
