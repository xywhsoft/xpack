@echo off
setlocal

echo ========================================
echo xpkgui 安装程序
echo ========================================
echo.

set INSTALL_DIR=%ProgramFiles%\xPack
set EXE_PATH=%INSTALL_DIR%\xpkgui.exe
set CLI_PATH=%INSTALL_DIR%\xpkcon.exe
set SHELL_DLL=%INSTALL_DIR%\xpkshext.dll
set XPK_DLL=%INSTALL_DIR%\xpack.dll

echo 安装目录: %INSTALL_DIR%
echo.

if not exist "%INSTALL_DIR%" (
    echo 创建安装目录...
    mkdir "%INSTALL_DIR%"
)

echo 检测系统架构...
if defined PROCESSOR_ARCHITEW6432 (
    set ARCH=x64
    set RELEASE_DIR=..\..\release\x64
) else if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
    set ARCH=x64
    set RELEASE_DIR=..\..\release\x64
) else (
    set ARCH=x86
    set RELEASE_DIR=..\..\release\x86
)

echo 检测到架构: %ARCH%
echo 源目录: %RELEASE_DIR%
echo.

if not exist "%RELEASE_DIR%\xpkgui.exe" (
    echo 错误: 未找到 %RELEASE_DIR%\xpkgui.exe
    echo 请先运行相应的编译脚本。
    pause
    exit /b 1
)

echo 复制文件...
copy "%RELEASE_DIR%\xpkgui.exe" "%EXE_PATH%" /Y >nul
if errorlevel 1 (
    echo 错误: 无法复制 xpkgui.exe
    pause
    exit /b 1
)
echo [OK] xpkgui.exe

if exist "%RELEASE_DIR%\xpkcon.exe" (
    copy "%RELEASE_DIR%\xpkcon.exe" "%CLI_PATH%" /Y >nul
    echo [OK] xpkcon.exe
) else (
    echo [警告] xpkcon.exe 未找到，跳过
)

if exist "%RELEASE_DIR%\xpkshext.dll" (
    copy "%RELEASE_DIR%\xpkshext.dll" "%SHELL_DLL%" /Y >nul
    if errorlevel 1 (
        echo [警告] 无法复制 xpkshext.dll
    ) else (
        echo [OK] xpkshext.dll
    )
)

if exist "%RELEASE_DIR%\xpack.dll" (
    copy "%RELEASE_DIR%\xpack.dll" "%XPK_DLL%" /Y >nul
    echo [OK] xpack.dll
) else (
    echo [警告] xpack.dll 未找到，跳过
)

echo.
echo 注册文件关联...

reg add HKCR\.xpk /ve /d xPack.File /f >nul
if errorlevel 1 (
    echo [警告] 无法注册 .xpk 文件类型
) else (
    echo [OK] .xpk 文件类型
)

reg add HKCR\xPack.File /ve /d "xPack 压缩包" /f >nul
reg add HKCR\xPack.File\DefaultIcon /ve /d "\"%EXE_PATH%\",0" /f >nul
reg add HKCR\xPack.File\shell\open\command /ve /d "\"%EXE_PATH%\" \"%%1\"" /f >nul

echo.
echo 注册 Shell 扩展...

if exist "%SHELL_DLL%" (
    regsvr32 /s "%SHELL_DLL%"
    if errorlevel 1 (
        echo [警告] Shell 扩展注册失败
    ) else (
        echo [OK] Shell 扩展
    )

    reg add HKCR\.xpk\shellex\ContextMenuHandlers\XPKShell /ve /d "{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}" /f >nul 2>nul
    reg add HKCR\*\shellex\ContextMenuHandlers\XPKShell /ve /d "{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}" /f >nul 2>nul
    reg add HKCR\Directory\shellex\ContextMenuHandlers\XPKShell /ve /d "{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}" /f >nul 2>nul
    reg add HKCR\Folder\shellex\ContextMenuHandlers\XPKShell /ve /d "{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}" /f >nul 2>nul
)

echo.
echo 刷新图标缓存...
ie4uinit.exe -show >nul 2>nul

echo.
echo ========================================
echo 安装完成！
echo ========================================
echo.
echo 如果安装了 Shell 扩展，请注销并重新登录
echo 以使右键菜单生效。
echo.
pause
