@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
set "ROOT_DIR=%SCRIPT_DIR%..\.."
for %%I in ("%ROOT_DIR%") do set "ROOT_DIR=%%~fI"
set "BIN_DIR=%ROOT_DIR%\release\x64"
set "DLL_PATH=%BIN_DIR%\xpkgui_shext.dll"

if exist "%DLL_PATH%" (
	echo Unregistering Explorer shell extension...
	regsvr32 /u /s "%DLL_PATH%"
)

reg delete "HKCU\Software\Classes\xPack.File" /f >nul 2>nul
reg delete "HKCU\Software\Classes\.xpk" /f >nul 2>nul

where ie4uinit.exe >nul 2>nul
if not errorlevel 1 (
	ie4uinit.exe -show >nul 2>nul
)

echo Uninstall complete.
exit /b 0
