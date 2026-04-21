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
	if errorlevel 1 (
		echo [ERROR] regsvr32 /u failed for %DLL_PATH%
		echo [ERROR] Exit code: %ERRORLEVEL%
		echo [HINT] Close Explorer windows and retry. For a visible COM unregister error dialog, run: regsvr32 /u "%DLL_PATH%"
		exit /b 1
	)
)

reg delete "HKCU\Software\Classes\xPack.File" /f >nul 2>nul
reg delete "HKCU\Software\Classes\.xpk" /f >nul 2>nul

where ie4uinit.exe >nul 2>nul
if not errorlevel 1 (
	ie4uinit.exe -show >nul 2>nul
)

echo Checking for residual per-user registration...
set "RESIDUAL_FOUND=0"
reg query "HKCU\Software\Classes\xPack.File" >nul 2>nul && set "RESIDUAL_FOUND=1"
reg query "HKCU\Software\Classes\.xpk" >nul 2>nul && set "RESIDUAL_FOUND=1"
reg query "HKCU\Software\Classes\CLSID\{7B8E8E8B-7C78-4C0F-BF9B-585F6F08D425}" >nul 2>nul && set "RESIDUAL_FOUND=1"
if "%RESIDUAL_FOUND%"=="1" (
	echo [WARN] Residual xpkgui registry entries were detected under HKCU\Software\Classes.
	echo [WARN] Run regedit and search for xPack.File, .xpk, or xpkgui_shext if Explorer still shows stale menu items.
)

echo Uninstall complete.
exit /b 0
