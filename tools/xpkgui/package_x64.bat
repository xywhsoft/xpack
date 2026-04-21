@echo off
setlocal EnableExtensions

set "SCRIPT_DIR=%~dp0"
set "ROOT_DIR=%SCRIPT_DIR%..\.."
for %%I in ("%ROOT_DIR%") do set "ROOT_DIR=%%~fI"
set "REL_DIR=%ROOT_DIR%\release\x64"
set "PKG_ROOT=%SCRIPT_DIR%obj\package"
set "PKG_DIR=%PKG_ROOT%\x64"

if not exist "%REL_DIR%\xpkgui.exe" (
	echo [ERROR] Missing %REL_DIR%\xpkgui.exe
	echo Run tools\xpkgui\build_x64.bat first.
	exit /b 1
)
if not exist "%REL_DIR%\xpkgui_shext.dll" (
	echo [ERROR] Missing %REL_DIR%\xpkgui_shext.dll
	echo Run tools\xpkgui\build_x64.bat first.
	exit /b 1
)

if exist "%PKG_DIR%" rmdir /s /q "%PKG_DIR%" || exit /b 1
mkdir "%PKG_DIR%\docs" || exit /b 1

copy /y "%REL_DIR%\xpkgui.exe" "%PKG_DIR%\" >nul || exit /b 1
copy /y "%REL_DIR%\xpkgui_shext.dll" "%PKG_DIR%\" >nul || exit /b 1
copy /y "%SCRIPT_DIR%install_x64.bat" "%PKG_DIR%\" >nul || exit /b 1
copy /y "%SCRIPT_DIR%uninstall_x64.bat" "%PKG_DIR%\" >nul || exit /b 1
copy /y "%SCRIPT_DIR%README.md" "%PKG_DIR%\" >nul || exit /b 1
copy /y "%SCRIPT_DIR%docs\USER_GUIDE.md" "%PKG_DIR%\docs\" >nul || exit /b 1
copy /y "%SCRIPT_DIR%docs\FAQ.md" "%PKG_DIR%\docs\" >nul || exit /b 1
copy /y "%SCRIPT_DIR%docs\RELEASE_CHECKLIST.md" "%PKG_DIR%\docs\" >nul || exit /b 1
powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%write_package_manifest.ps1" "%PKG_DIR%" || exit /b 1

echo xpkgui package staged:
echo   %PKG_DIR%
exit /b 0
