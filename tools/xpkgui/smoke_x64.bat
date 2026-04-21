@echo off
setlocal EnableExtensions

set "SCRIPT_DIR=%~dp0"
set "ROOT_DIR=%SCRIPT_DIR%..\.."
for %%I in ("%ROOT_DIR%") do set "ROOT_DIR=%%~fI"
set "REL_DIR=%ROOT_DIR%\release\x64"
set "XPKCON=%REL_DIR%\xpkcon.exe"
set "XPKGUI=%REL_DIR%\xpkgui.exe"
set "XPKSHEXT=%REL_DIR%\xpkgui_shext.dll"
set "WORK_DIR=%SCRIPT_DIR%obj\smoke"
set "INPUT_DIR=%WORK_DIR%\input"
set "OUT_DIR=%WORK_DIR%\out"
set "EXTRACT_DIR=%WORK_DIR%\extract"
set "XPKGUI_SMOKE=1"

echo [smoke] Building xpkgui...
call "%SCRIPT_DIR%build_x64.bat" || exit /b 1

if not exist "%XPKGUI%" (
	echo [ERROR] xpkgui.exe was not produced: %XPKGUI%
	exit /b 1
)

if not exist "%XPKSHEXT%" (
	echo [ERROR] xpkgui_shext.dll was not produced: %XPKSHEXT%
	exit /b 1
)

if not exist "%XPKCON%" (
	echo [ERROR] xpkcon.exe is required for smoke validation: %XPKCON%
	exit /b 1
)

call :static_checks || exit /b 1

if exist "%WORK_DIR%" rmdir /s /q "%WORK_DIR%" || exit /b 1
mkdir "%INPUT_DIR%\sub" "%OUT_DIR%" "%EXTRACT_DIR%" || exit /b 1

> "%INPUT_DIR%\alpha.txt" echo alpha
> "%INPUT_DIR%\sub\beta.txt" echo beta
> "%INPUT_DIR%\readme.md" echo # smoke

pushd "%WORK_DIR%" || exit /b 1

call :run "%XPKCON%" a "out\core.xpk" -tcore "input\alpha.txt" "input\sub\beta.txt" "input\readme.md" || goto :failed
call :run "%XPKCON%" a "out\index.xpk" -tindex 100::"input\alpha.txt" 101::"input\sub\beta.txt" 102::"input\readme.md" || goto :failed
call :run "%XPKCON%" a "out\linux.xpk" -tlinux docs/alpha.txt::"input\alpha.txt" docs/sub/beta.txt::"input\sub\beta.txt" docs/readme.md::"input\readme.md" || goto :failed
call :run "%XPKCON%" a "out\win32.xpk" -twin32 docs/alpha.txt::"input\alpha.txt" docs/sub/beta.txt::"input\sub\beta.txt" docs/readme.md::"input\readme.md" || goto :failed

for %%P in (core index linux win32) do (
	call :validate "%%P" || goto :failed
)

call :validate_cli || goto :failed

popd
echo.
echo [smoke] OK: generated and validated packages under %OUT_DIR%
echo [smoke] GUI binary: %XPKGUI%
exit /b 0

:failed
set "SMOKE_CODE=%ERRORLEVEL%"
popd >nul 2>nul
if "%SMOKE_CODE%"=="0" set "SMOKE_CODE=1"
exit /b %SMOKE_CODE%

:validate
set "PKG_NAME=%~1"
call :run "%XPKCON%" i "out\%PKG_NAME%.xpk" || exit /b 1
call :run "%XPKCON%" l "out\%PKG_NAME%.xpk" || exit /b 1
call :run "%XPKCON%" t "out\%PKG_NAME%.xpk" || exit /b 1
mkdir "extract\%PKG_NAME%" >nul 2>nul
call :run "%XPKCON%" e "out\%PKG_NAME%.xpk" -o"extract\%PKG_NAME%" || exit /b 1
exit /b 0

:validate_cli
echo [smoke] Validating xpkgui command-line entries...
echo [smoke] "%XPKGUI%" /verify "out\core.xpk"
"%XPKGUI%" /verify "out\core.xpk" || exit /b 1
echo [smoke] "%XPKGUI%" /properties "out\core.xpk"
"%XPKGUI%" /properties "out\core.xpk" || exit /b 1
mkdir "extract\cli_explicit" >nul 2>nul
echo [smoke] "%XPKGUI%" /extract "out\linux.xpk" "extract\cli_explicit"
"%XPKGUI%" /extract "out\linux.xpk" "extract\cli_explicit" || exit /b 1
echo [smoke] "%XPKGUI%" /extract-auto "out\index.xpk"
"%XPKGUI%" /extract-auto "out\index.xpk" || exit /b 1
echo [smoke] "%XPKGUI%" /extract-here "out\win32.xpk"
"%XPKGUI%" /extract-here "out\win32.xpk" || exit /b 1
if exist "input\xpack_archive.xpk" del /q "input\xpack_archive.xpk" || exit /b 1
echo [smoke] "%XPKGUI%" /shell-add-auto "input\alpha.txt" "input\sub\beta.txt"
"%XPKGUI%" /shell-add-auto "input\alpha.txt" "input\sub\beta.txt" || exit /b 1
if not exist "input\xpack_archive.xpk" (
	echo [ERROR] /shell-add-auto did not create input\xpack_archive.xpk
	exit /b 1
)
call :run "%XPKCON%" t "input\xpack_archive.xpk" || exit /b 1
echo [smoke] expect-fail "%XPKGUI%" /definitely-unknown-option
"%XPKGUI%" /definitely-unknown-option
if not errorlevel 1 (
	echo [ERROR] Command unexpectedly succeeded: "%XPKGUI%" /definitely-unknown-option
	exit /b 1
)
exit /b 0

:static_checks
echo [smoke] Running static checks...
if not exist "%SCRIPT_DIR%install_x64.bat" (
	echo [ERROR] install_x64.bat is missing.
	exit /b 1
)
if not exist "%SCRIPT_DIR%uninstall_x64.bat" (
	echo [ERROR] uninstall_x64.bat is missing.
	exit /b 1
)
findstr /C:"regsvr32" "%SCRIPT_DIR%install_x64.bat" >nul || (
	echo [ERROR] install_x64.bat does not reference regsvr32.
	exit /b 1
)
findstr /C:"xpkgui_shext.dll" "%SCRIPT_DIR%install_x64.bat" >nul || (
	echo [ERROR] install_x64.bat does not reference xpkgui_shext.dll.
	exit /b 1
)
findstr /C:"#define XPKGUI_ARCHIVE_COLUMN_COUNT 10" "%SCRIPT_DIR%src\app.h" >nul || (
	echo [ERROR] XPKGUI_ARCHIVE_COLUMN_COUNT is not 10.
	exit /b 1
)
findstr /C:"ColumnCount" "%SCRIPT_DIR%src\common.c" >nul || (
	echo [ERROR] ColumnCount settings migration checks are missing.
	exit /b 1
)
findstr /C:"Ratio" "%SCRIPT_DIR%src\archive.c" >nul || (
	echo [ERROR] Ratio list column support is missing.
	exit /b 1
)
exit /b 0

:run
echo [smoke] %*
%*
if errorlevel 1 (
	echo [ERROR] Command failed: %*
	exit /b 1
)
exit /b 0

:run_expect_fail
echo [smoke] expect-fail %*
%*
if errorlevel 1 (
	exit /b 0
)
echo [ERROR] Command unexpectedly succeeded: %*
exit /b 1
