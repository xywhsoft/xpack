@echo off
setlocal

echo ========================================
echo xpkgui 卸载程序
echo ========================================
echo.

set INSTALL_DIR=%ProgramFiles%\xPack
set EXE_PATH=%INSTALL_DIR%\xpkgui.exe
set SHELL_DLL=%INSTALL_DIR%\xpkshext.dll

echo 安装目录: %INSTALL_DIR%
echo.

echo 注销 Shell 扩展...
if exist "%SHELL_DLL%" (
    regsvr32 /u /s "%SHELL_DLL%"
    if errorlevel 1 (
        echo [警告] 无法注销 Shell 扩展
    ) else (
        echo [OK] Shell 扩展已注销
    )

    reg delete HKCR\.xpk\shellex\ContextMenuHandlers\XPKShell /f >nul 2>nul
    reg delete HKCR\*\shellex\ContextMenuHandlers\XPKShell /f >nul 2>nul
    reg delete HKCR\Directory\shellex\ContextMenuHandlers\XPKShell /f >nul 2>nul
    reg delete HKCR\Folder\shellex\ContextMenuHandlers\XPKShell /f >nul 2>nul
) else (
    echo [提示] xpkshext.dll 未找到，跳过 Shell Extension 卸载
)

echo.
echo 注销文件关联...
reg delete HKCR\.xpk /f >nul 2>nul
reg delete HKCR\xPack.File /f >nul 2>nul

if errorlevel 1 (
    echo [警告] 无法注销文件关联
) else (
    echo [OK] 文件关联已注销
)

echo.
echo 删除文件...
if exist "%EXE_PATH%" (
    del "%EXE_PATH%" /F /Q
    echo [OK] xpkgui.exe
) else (
    echo [警告] xpkgui.exe 未找到
)

if exist "%SHELL_DLL%" (
    del "%SHELL_DLL%" /F /Q
    echo [OK] xpkshext.dll
) else (
    echo [提示] xpkshext.dll 未找到
)

echo.
echo 检查是否需要删除安装目录...
dir "%INSTALL_DIR%" /b >nul 2>nul
if errorlevel 1 (
    rd "%INSTALL_DIR%"
    echo [OK] 安装目录已删除
) else (
    echo [提示] 安装目录中还有其他文件，未删除
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
