@echo off
setlocal enabledelayedexpansion

echo =================================================
echo   xpkcon Full Test Suite
echo =================================================
echo.

set OUTPUT_DIR=..\..\..\release\x64
set TEST_EXE=%OUTPUT_DIR%\test_all.exe

if not exist "%TEST_EXE%" (
    echo Test executable not found, building...
    call build_test.bat
    if !ERRORLEVEL! NEQ 0 (
        echo Build failed!
        exit /b 1
    )
    echo.
)

echo Running full test suite...
echo Options: --html  Generate HTML report
echo          --json  Generate JSON report
echo.
echo Test executable: %TEST_EXE%
echo.

set CMD="%TEST_EXE%"

if "%~1"=="--html" set CMD=!CMD! --html
if "%~1"=="--json" set CMD=!CMD! --json

if "%~2"=="--html" set CMD=!CMD! --html
if "%~2"=="--json" set CMD=!CMD! --json

!CMD!

set RESULT=!ERRORLEVEL!

echo.

if !RESULT! EQU 0 (
    echo All tests passed!
) else (
    echo Some tests failed!
)

exit /b !RESULT!
