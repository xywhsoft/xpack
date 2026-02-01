@echo off
echo Building xpkcon test programs...
echo.

set OUTPUT_DIR=..\..\..\release\x64
set OUTPUT1=%OUTPUT_DIR%\test_xpkcon.exe
set OUTPUT2=%OUTPUT_DIR%\test_all.exe

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

gcc -o "%OUTPUT1%" ^
	test_xpkcon.c ^
	test_cases.c ^
	test_framework.c ^
	-I../.. ^
	-O2 -Wall

if %ERRORLEVEL% NEQ 0 (
    echo Build of test_xpkcon failed!
    exit /b 1
)

gcc -o "%OUTPUT2%" ^
	test_all.c ^
	test_cases.c ^
	test_framework.c ^
	-I../.. ^
	-O2 -Wall

if %ERRORLEVEL% EQU 0 (
	echo.
	echo Build successful!
	echo Output: %OUTPUT1%
	echo         %OUTPUT2%
) else (
	echo.
	echo Build of test_all failed!
)
