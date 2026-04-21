@echo off
setlocal EnableExtensions

set "SCRIPT_DIR=%~dp0"
set "ROOT_DIR=%SCRIPT_DIR%..\.."
for %%I in ("%ROOT_DIR%") do set "ROOT_DIR=%%~fI"
set "REL_DIR=%ROOT_DIR%\release\x64"
set "STRICT=0"
set "WARN_COUNT=0"

if /I "%~1"=="/strict" set "STRICT=1"

if not exist "%REL_DIR%" (
	echo [ERROR] Missing release directory: %REL_DIR%
	exit /b 1
)

echo [release-check] Auditing %REL_DIR%
for /f "delims=" %%F in ('dir /b /a "%REL_DIR%" 2^>nul') do call :classify "%%F"

if "%WARN_COUNT%"=="0" (
	echo [release-check] OK: no unexpected xpkgui release payload detected.
	exit /b 0
)

echo [release-check] WARN: %WARN_COUNT% unexpected item(s) are present.
echo [release-check] See tools\xpkgui\docs\RELEASE_MANIFEST.md for the expected payload.
if "%STRICT%"=="1" exit /b 1
exit /b 0

:classify
set "ITEM=%~1"
if /I "%ITEM%"=="xpkgui.exe" exit /b 0
if /I "%ITEM%"=="xpkgui_shext.dll" exit /b 0
if /I "%ITEM%"=="xpkcon.exe" exit /b 0
if /I "%ITEM%"=="xpack.exe" exit /b 0
if /I "%ITEM%"=="xpack.dll" exit /b 0
echo [release-check] WARN: not xpkgui release payload: %ITEM%
set /a WARN_COUNT+=1
exit /b 0
