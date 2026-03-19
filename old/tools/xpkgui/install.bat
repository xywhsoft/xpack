@echo off
setlocal

echo ========================================
echo xpkgui 安装程序
echo ========================================
echo.

set INSTALL_DIR=%ProgramFiles%\xPack
set EXE_PATH=%INSTALL_DIR%\xpkgui.exe
set SHELL_DLL=%INSTALL_DIR%\xpkshext.dll

echo 安装目录: %INSTALL_DIR%
echo.

if not exist "%INSTALL_DIR%" (
    echo 创建安装目录...
    mkdir "%INSTALL_DIR%"
)

echo 检测系统架构...
if defined PROCESSOR_ARCHITEW6432 (
    set ARCH=x64
    set SHELL_BUILD_DIR=src\shell\build_x64
) else if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
    set ARCH=x64
    set SHELL_BUILD_DIR=src\shell\build_x64
) else (
    set ARCH=x86
    set SHELL_BUILD_DIR=src\shell\build_x86
)

echo 检测到架构: %ARCH%
echo.

if not exist "bin\xpkgui.exe" (
    echo 错误: 未找到 bin\xpkgui.exe
    echo 请先运行 build.bat 编译程序。
    pause
    exit /b 1
)

echo 复制文件...
copy "bin\xpkgui.exe" "%EXE_PATH%" /Y >nul
if errorlevel 1 (
    echo 错误: 无法复制 xpkgui.exe
    pause
    exit /b 1
)
echo [OK] xpkgui.exe

if exist "%SHELL_BUILD_DIR%\xpkshext.dll" (
    copy "%SHELL_BUILD_DIR%\xpkshext.dll" "%SHELL_DLL%" /Y >nul
    if errorlevel 1 (
        echo [警告] 无法复制 xpkshext.dll
    ) else (
        echo [OK] xpkshext.dll
    )
) else (
    echo [警告] xpkshext.dll 未找到，跳过 Shell Extension 安装
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
reg add HKCR\xPack.File\shell\manage /ve /d "使用 xpkgui 管理文件" /f >nul
reg add HKCR\xPack.File\shell\manage\command /ve /d "\"%EXE_PATH%\" \"%%1\"" /f >nul

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
