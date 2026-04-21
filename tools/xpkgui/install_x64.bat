@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
set "ROOT_DIR=%SCRIPT_DIR%..\.."
for %%I in ("%ROOT_DIR%") do set "ROOT_DIR=%%~fI"
set "BIN_DIR=%ROOT_DIR%\release\x64"
set "EXE_PATH=%BIN_DIR%\xpkgui.exe"
set "DLL_PATH=%BIN_DIR%\xpkgui_shext.dll"

if not exist "%EXE_PATH%" (
	echo [ERROR] Missing %EXE_PATH%
	echo Run tools\xpkgui\build_x64.bat first.
	exit /b 1
)
if not exist "%DLL_PATH%" (
	echo [ERROR] Missing %DLL_PATH%
	echo Run tools\xpkgui\build_x64.bat first.
	exit /b 1
)

echo Registering Explorer shell extension...
regsvr32 /s "%DLL_PATH%"
if errorlevel 1 (
	echo [ERROR] regsvr32 failed for %DLL_PATH%
	echo [ERROR] Exit code: %ERRORLEVEL%
	echo [HINT] Close Explorer windows, make sure the DLL exists and matches x64, then retry.
	echo [HINT] For a visible COM registration error dialog, run: regsvr32 "%DLL_PATH%"
	exit /b 1
)

echo Registering per-user .xpk association...
reg add "HKCU\Software\Classes\.xpk" /ve /d "xPack.File" /f >nul || exit /b 1
reg add "HKCU\Software\Classes\xPack.File" /ve /d "xPack Archive" /f >nul || exit /b 1
reg add "HKCU\Software\Classes\xPack.File\DefaultIcon" /ve /d "\"%EXE_PATH%\",0" /f >nul || exit /b 1
reg add "HKCU\Software\Classes\xPack.File\shell\open" /ve /d "Open with xpkgui" /f >nul || exit /b 1
reg add "HKCU\Software\Classes\xPack.File\shell\open\command" /ve /d "\"%EXE_PATH%\" \"%%1\"" /f >nul || exit /b 1
reg add "HKCU\Software\Classes\xPack.File\shell\extract" /ve /d "Extract..." /f >nul || exit /b 1
reg add "HKCU\Software\Classes\xPack.File\shell\extract\command" /ve /d "\"%EXE_PATH%\" /extract \"%%1\"" /f >nul || exit /b 1
reg add "HKCU\Software\Classes\xPack.File\shell\extract_auto" /ve /d "Extract to Folder" /f >nul || exit /b 1
reg add "HKCU\Software\Classes\xPack.File\shell\extract_auto\command" /ve /d "\"%EXE_PATH%\" /extract-auto \"%%1\"" /f >nul || exit /b 1
reg add "HKCU\Software\Classes\xPack.File\shell\extract_here" /ve /d "Extract Here" /f >nul || exit /b 1
reg add "HKCU\Software\Classes\xPack.File\shell\extract_here\command" /ve /d "\"%EXE_PATH%\" /extract-here \"%%1\"" /f >nul || exit /b 1
reg add "HKCU\Software\Classes\xPack.File\shell\verify" /ve /d "Verify Archive" /f >nul || exit /b 1
reg add "HKCU\Software\Classes\xPack.File\shell\verify\command" /ve /d "\"%EXE_PATH%\" /verify \"%%1\"" /f >nul || exit /b 1
reg add "HKCU\Software\Classes\xPack.File\shell\properties" /ve /d "Archive Properties" /f >nul || exit /b 1
reg add "HKCU\Software\Classes\xPack.File\shell\properties\command" /ve /d "\"%EXE_PATH%\" /properties \"%%1\"" /f >nul || exit /b 1

where ie4uinit.exe >nul 2>nul
if not errorlevel 1 (
	ie4uinit.exe -show >nul 2>nul
)

echo Installation complete.
echo Explorer may need to refresh before the new menu appears.
exit /b 0
