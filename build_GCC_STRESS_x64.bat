@echo off
setlocal EnableDelayedExpansion

cd /d "%~dp0"

if /I "%~1"=="help" goto :help
if /I "%~1"=="-h" goto :help
if /I "%~1"=="--help" goto :help

set "iRounds=%~1"
if "%iRounds%"=="" set "iRounds=5"

set "sFilter=%~2"
if "%sFilter%"=="" set "sFilter=integration/stress/direct"

set "iInnerRepeat=%~3"

set "sLogLatest=release\x64\xpack_stress.log"
set "sLock=release\x64\xpack_stress.lock"
set "sHistory=release\x64\xpack_stress_history.tsv"
set "sStamp="
set "iStartUnix="
set "iEndUnix="
set "iElapsed="
set "sLog="

if exist "%sLock%" (
	echo Stress lock exists: %sLock%
	echo Another stress run is probably active.
	exit /b 3
)

for /f %%i in ('powershell -NoProfile -Command "(Get-Date).ToString(\"yyyyMMdd_HHmmss\")"') do set "sStamp=%%i"
if "%sStamp%"=="" set "sStamp=unknown"
set "sLog=release\x64\xpack_stress_%sStamp%.log"

if not exist "%sHistory%" (
	> "%sHistory%" echo stamp	filter	outer_rounds	inner_stress_repeat	result	elapsed_seconds	log
)

for /f %%i in ('powershell -NoProfile -Command "[DateTimeOffset]::Now.ToUnixTimeSeconds()"') do set "iStartUnix=%%i"
if "%iStartUnix%"=="" set "iStartUnix=0"

echo %DATE% %TIME% > "%sLock%"
echo log=%sLog%>> "%sLock%"

call build_GCC_TEST_x64.bat
if errorlevel 1 (
	del /q "%sLock%" >nul 2>nul
	exit /b %errorlevel%
)

echo xPack stress start > "%sLog%"
echo stamp=%sStamp% >> "%sLog%"
echo filter=%sFilter% >> "%sLog%"
echo rounds=%iRounds% >> "%sLog%"
if not "%iInnerRepeat%"=="" echo stress_repeat=%iInnerRepeat% >> "%sLog%"
echo start_date=%DATE% >> "%sLog%"
echo start_time=%TIME% >> "%sLog%"
echo start_unix=%iStartUnix% >> "%sLog%"
echo. >> "%sLog%"

echo Running %iRounds% round(s) with filter %sFilter% ...
if not "%iInnerRepeat%"=="" echo Inner stress repeat %iInnerRepeat%

for /L %%i in (1,1,%iRounds%) do (
	echo [%%i/%iRounds%] %sFilter%
	echo [%%i/%iRounds%] %sFilter% >> "%sLog%"
	if "%iInnerRepeat%"=="" (
		release\x64\xpack_test.exe %sFilter% >> "%sLog%" 2>&1
	) else (
		release\x64\xpack_test.exe %sFilter% %iInnerRepeat% >> "%sLog%" 2>&1
	)
	if errorlevel 1 (
		set "iFailCode=!errorlevel!"
		echo.
		echo Stress failed at round %%i, exit code !iFailCode!
		echo FAILED round=%%i exit=!iFailCode! >> "%sLog%"
		for /f %%j in ('powershell -NoProfile -Command "[DateTimeOffset]::Now.ToUnixTimeSeconds()"') do set "iEndUnix=%%j"
		if not "!iEndUnix!"=="" if not "%iStartUnix%"=="0" set /a iElapsed=!iEndUnix!-%iStartUnix%
		if not "!iEndUnix!"=="" echo end_unix=!iEndUnix! >> "%sLog%"
		if defined iElapsed echo elapsed_seconds=!iElapsed! >> "%sLog%"
		copy /y "%sLog%" "%sLogLatest%" >nul
		>> "%sHistory%" echo %sStamp%	%sFilter%	%iRounds%	%iInnerRepeat%	FAILED_!iFailCode!	!iElapsed!	%sLog%
		del /q "%sLock%" >nul 2>nul
		exit /b !iFailCode!
	)
)

echo.
echo Stress successful: %iRounds% round(s)
echo SUCCESS >> "%sLog%"
for /f %%i in ('powershell -NoProfile -Command "[DateTimeOffset]::Now.ToUnixTimeSeconds()"') do set "iEndUnix=%%i"
if not "%iEndUnix%"=="" if not "%iStartUnix%"=="0" set /a iElapsed=%iEndUnix%-%iStartUnix%
if not "%iEndUnix%"=="" echo end_unix=%iEndUnix% >> "%sLog%"
if defined iElapsed echo elapsed_seconds=%iElapsed% >> "%sLog%"
copy /y "%sLog%" "%sLogLatest%" >nul
>> "%sHistory%" echo %sStamp%	%sFilter%	%iRounds%	%iInnerRepeat%	SUCCESS	%iElapsed%	%sLog%
del /q "%sLock%" >nul 2>nul
exit /b 0

:help
echo Usage:
echo   build_GCC_STRESS_x64.bat [outer_rounds] [filter] [inner_stress_repeat]
echo.
echo Defaults:
echo   outer_rounds=5
echo   filter=integration/stress/direct
echo   inner_stress_repeat=use xpack_test default ^(5^) unless provided
echo.
echo Examples:
echo   build_GCC_STRESS_x64.bat
echo   build_GCC_STRESS_x64.bat 3
echo   build_GCC_STRESS_x64.bat 2 integration/stress/direct 6
echo   build_GCC_STRESS_x64.bat 1 all 2
echo.
echo Notes:
echo   This script is not parallel-safe.
echo   A lock file is created at release\x64\xpack_stress.lock during execution.
echo   A timestamped log is written to release\x64\xpack_stress_YYYYMMDD_HHMMSS.log
echo   The latest run is also copied to release\x64\xpack_stress.log
echo   A history row is appended to release\x64\xpack_stress_history.tsv
exit /b 0
