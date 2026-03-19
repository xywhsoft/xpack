@echo off
echo ====================================
echo   xpkcon Build Helper
echo ====================================
echo.
echo Please select build configuration:
echo.
echo [1] TCC x64 Static    (Standalone, no DLL needed)
echo [2] TCC x64 Dynamic   (Needs xpack.dll)
echo [3] TCC x86 Static    (Standalone, no DLL needed)
echo [4] TCC x86 Dynamic   (Needs xpack.dll)
echo [5] GCC x64 Static    (Standalone, no DLL needed)
echo [6] GCC x64 Dynamic   (Needs xpack.dll)
echo [7] GCC x86 Static    (Standalone, no DLL needed)
echo [8] GCC x86 Dynamic   (Needs xpack.dll)
echo.
echo [0] Exit
echo.

set /p choice="Enter your choice (0-8): "

if "%choice%"=="1" goto tcc_static_x64
if "%choice%"=="2" goto tcc_dynamic_x64
if "%choice%"=="3" goto tcc_static_x86
if "%choice%"=="4" goto tcc_dynamic_x86
if "%choice%"=="5" goto gcc_static_x64
if "%choice%"=="6" goto gcc_dynamic_x64
if "%choice%"=="7" goto gcc_static_x86
if "%choice%"=="8" goto gcc_dynamic_x86
if "%choice%"=="0" goto end

echo Invalid choice!
goto end

:tcc_static_x64
call build_tcc_static_x64.bat
goto end

:tcc_dynamic_x64
call build_tcc_dynamic_x64.bat
goto end

:tcc_static_x86
call build_tcc_static_x86.bat
goto end

:tcc_dynamic_x86
call build_tcc_dynamic_x86.bat
goto end

:gcc_static_x64
call build_gcc_static_x64.bat
goto end

:gcc_dynamic_x64
call build_gcc_dynamic_x64.bat
goto end

:gcc_static_x86
call build_gcc_static_x86.bat
goto end

:gcc_dynamic_x86
call build_gcc_dynamic_x86.bat
goto end

:end
echo.
pause
