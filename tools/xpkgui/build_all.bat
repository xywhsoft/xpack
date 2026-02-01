@echo off
setlocal EnableDelayedExpansion

echo ========================================
echo xPack GUI 工具完整构建系统
echo ========================================
echo.

REM 检测系统架构
if defined PROCESSOR_ARCHITEW6432 (
    set ARCH=x64
) else if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
    set ARCH=x64
) else (
    set ARCH=x86
)

echo 检测到系统架构: %ARCH%
echo.

REM 检查编译器
set TCC_FOUND=0
set GCC_FOUND=0

where tcc >nul 2>&1
if !errorlevel! equ 0 (
    set TCC_FOUND=1
    echo [OK] 找到 TCC 编译器
)

where gcc >nul 2>&1
if !errorlevel! equ 0 (
    set GCC_FOUND=1
    echo [OK] 找到 GCC 编译器
)

echo.

REM 构建选择
if "%1"=="" (
    echo 使用方法:
    echo   build_all.bat [选项]
    echo.
    echo 选项:
    echo   dll        - 构建 xpack.dll (必需)
    echo   xpkgui     - 构建 xpkgui (所有变体)
    echo   xpkcon     - 构建 xpkcon (所有变体)
    echo   shext      - 构建 xpkshext.dll (Shell 扩展)
    echo   all        - 构建所有组件
    echo   clean      - 清理构建输出
    echo.
    echo 默认: all
    echo.
    set BUILD_TARGET=all
) else (
    set BUILD_TARGET=%1
)

echo ========================================
echo 开始构建: %BUILD_TARGET%
echo ========================================
echo.

REM 清理
if "%BUILD_TARGET%"=="clean" (
    echo 清理构建输出...
    if exist "..\..\release\x64" (
        rmdir /s /q "..\..\release\x64"
        echo [OK] 已删除 release\x64
    )
    if exist "..\..\release\x86" (
        rmdir /s /q "..\..\release\x86"
        echo [OK] 已删除 release\x86
    )
    if exist "..\..\release\linux" (
        rmdir /s /q "..\..\release\linux"
        echo [OK] 已删除 release\linux
    )
    echo 清理完成!
    exit /b 0
)

REM 构建 DLL
if "%BUILD_TARGET%"=="dll" or "%BUILD_TARGET%"=="all" (
    echo [1/4] 构建 xpack.dll
    if exist "..\..\build_TCC_DLL_x64.bat" (
        call "..\..\build_TCC_DLL_x64.bat"
        if !errorlevel! neq 0 (
            echo [警告] TCC x64 DLL 构建失败
        )
    )
    echo.
)

REM 构建 xpkgui
if "%BUILD_TARGET%"=="xpkgui" or "%BUILD_TARGET%"=="all" (
    echo [2/4] 构建 xpkgui
    
    if %TCC_FOUND%==1 (
        echo   - TCC x64 动态链接...
        call build_tcc_dynamic_x64.bat
        echo.
        
        echo   - TCC x64 静态链接...
        call build_tcc_static_x64.bat
        echo.
    )
    
    if %GCC_FOUND%==1 (
        echo   - GCC x64 动态链接...
        call build_gcc_dynamic_x64.bat
        echo.
        
        echo   - GCC x64 静态链接...
        call build_gcc_static_x64.bat
        echo.
    )
    
    echo [OK] xpkgui 构建完成
    echo.
)

REM 构建 xpkcon
if "%BUILD_TARGET%"=="xpkcon" or "%BUILD_TARGET%"=="all" (
    echo [3/4] 构建 xpkcon
    cd ..\xpkcon
    
    if %TCC_FOUND%==1 (
        echo   - TCC x64 动态链接...
        call build_tcc_dynamic_x64.bat
        echo.
        
        echo   - TCC x64 静态链接...
        call build_tcc_static_x64.bat
        echo.
    )
    
    if %GCC_FOUND%==1 (
        echo   - GCC x64 动态链接...
        call build_gcc_dynamic_x64.bat
        echo.
        
        echo   - GCC x64 静态链接...
        call build_gcc_static_x64.bat
        echo.
    )
    
    cd ..\xpkgui
    echo [OK] xpkcon 构建完成
    echo.
)

REM 构建 Shell 扩展
if "%BUILD_TARGET%"=="shext" or "%BUILD_TARGET%"=="all" (
    echo [4/4] 构建 xpkshext.dll
    
    if %TCC_FOUND%==1 (
        echo   - TCC x64...
        call build_shext_tcc_x64.bat
        echo.
    )
    
    if %GCC_FOUND%==1 (
        echo   - GCC x64...
        call build_shext_gcc_x64.bat
        echo.
    )
    
    echo [OK] xpkshext.dll 构建完成
    echo.
)

echo ========================================
echo 构建完成!
echo ========================================
echo.
echo 输出目录:
echo   x64: ..\..\release\x64\
echo   x86: ..\..\release\x86\
echo.
echo 安装: 运行 install.bat
echo 卸载: 运行 uninstall.bat
echo.
