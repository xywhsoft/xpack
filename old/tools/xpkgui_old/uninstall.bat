@echo off
setlocal

echo ========================================
echo xpkgui 卸载程序
echo ========================================
echo.

set INSTALL_DIR=%ProgramFiles%\xPack
set EXE_PATH=%INSTALL_DIR%\xpkgui.exe
set CLI_PATH=%INSTALL_DIR%\xpkcon.exe
set SHELL_DLL=%INSTALL_DIR%\xpkshext.dll
set XPK_DLL=%INSTALL_DIR%\xpack.dll

echo 卸载目录: %INSTALL_DIR%
echo.

echo 取消注册 Shell 扩展...

if exist "%SHELL_DLL%" (
    regsvr32 /u /s "%SHELL_DLL%"
    echo [OK] Shell 扩展已注销
)

echo.
echo 删除右键菜单注册表项...

reg delete HKCR\.xpk\shellex\ContextMenuHandlers\XPKShell /f >nul 2>nul
reg delete HKCR\*\shellex\ContextMenuHandlers\XPKShell /f >nul 2>nul
reg delete HKCR\Directory\shellex\ContextMenuHandlers\XPKShell /f >nul 2>nul
reg delete HKCR\Folder\shellex\ContextMenuHandlers\XPKShell /f >nul 2>nul

echo [OK] 右键菜单已删除

echo.
echo 删除文件关联...

reg delete HKCR\.xpk /f >nul 2>nul
reg delete HKCR\xPack.File /f >nul 2>nul

echo [OK] 文件关联已删除

echo.
echo 删除程序文件...

if exist "%EXE_PATH%" (
    del "%EXE_PATH%"
    echo [OK] xpkgui.exe 已删除
)

if exist "%CLI_PATH%" (
    del "%CLI_PATH%"
    echo [OK] xpkcon.exe 已删除
)

if exist "%SHELL_DLL%" (
    del "%SHELL_DLL%"
    echo [OK] xpkshext.dll 已删除
)

if exist "%XPK_DLL%" (
    del "%XPK_DLL%"
    echo [OK] xpack.dll 已删除
)

echo.
echo 删除配置目录...

set CONFIG_DIR=%APPDATA%\xPack
if exist "%CONFIG_DIR%" (
    rmdir /s /q "%CONFIG_DIR%"
    echo [OK] 配置目录已删除
)

echo.
echo 刷新图标缓存...
ie4uinit.exe -show >nul 2>nul

echo.
echo ========================================
echo 卸载完成！
echo ========================================
echo.
pause
