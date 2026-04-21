@echo off
setlocal EnableExtensions

set "SCRIPT_DIR=%~dp0"
set "ROOT_DIR=%SCRIPT_DIR%..\.."
for %%I in ("%ROOT_DIR%") do set "ROOT_DIR=%%~fI"
set "REL_DIR=%ROOT_DIR%\release\x64"
set "XPKCON=%REL_DIR%\xpkcon.exe"
set "XPKGUI=%REL_DIR%\xpkgui.exe"
set "XPKSHEXT=%REL_DIR%\xpkgui_shext.dll"
set "RUN_TIMEOUT=%SCRIPT_DIR%run_with_timeout.ps1"
set "WORK_DIR=%SCRIPT_DIR%obj\smoke"
set "INPUT_DIR=%WORK_DIR%\input"
set "OUT_DIR=%WORK_DIR%\out"
set "EXTRACT_DIR=%WORK_DIR%\extract"
set "PKG_DIR=%SCRIPT_DIR%obj\package\x64"
set "RELEASE_AUDIT_REPORT=%SCRIPT_DIR%obj\release_audit_x64.md"
set "XPKGUI_SMOKE=1"

echo [smoke] Building xpkgui...
call "%SCRIPT_DIR%build_x64.bat" || exit /b 1

if not exist "%XPKGUI%" (
	echo [ERROR] xpkgui.exe was not produced: %XPKGUI%
	exit /b 1
)

if not exist "%XPKSHEXT%" (
	echo [ERROR] xpkgui_shext.dll was not produced: %XPKSHEXT%
	exit /b 1
)

if not exist "%XPKCON%" (
	echo [ERROR] xpkcon.exe is required for smoke validation: %XPKCON%
	exit /b 1
)

call "%SCRIPT_DIR%check_release_x64.bat" || exit /b 1
call :write_release_audit_report || exit /b 1
call :static_checks || exit /b 1
call "%SCRIPT_DIR%package_x64.bat" || exit /b 1
call :validate_package || exit /b 1

if exist "%WORK_DIR%" rmdir /s /q "%WORK_DIR%" || exit /b 1
mkdir "%INPUT_DIR%\sub" "%OUT_DIR%" "%EXTRACT_DIR%" || exit /b 1

> "%INPUT_DIR%\alpha.txt" echo alpha
> "%INPUT_DIR%\sub\beta.txt" echo beta
> "%INPUT_DIR%\readme.md" echo # smoke

pushd "%WORK_DIR%" || exit /b 1

call :run "%XPKCON%" a "out\core.xpk" -tcore "input\alpha.txt" "input\sub\beta.txt" "input\readme.md" || goto :failed
call :run "%XPKCON%" a "out\index.xpk" -tindex 100::"input\alpha.txt" 101::"input\sub\beta.txt" 102::"input\readme.md" || goto :failed
call :run "%XPKCON%" a "out\linux.xpk" -tlinux docs/alpha.txt::"input\alpha.txt" docs/sub/beta.txt::"input\sub\beta.txt" docs/readme.md::"input\readme.md" || goto :failed
call :run "%XPKCON%" a "out\win32.xpk" -twin32 docs/alpha.txt::"input\alpha.txt" docs/sub/beta.txt::"input\sub\beta.txt" docs/readme.md::"input\readme.md" || goto :failed

for %%P in (core index linux win32) do (
	call :validate "%%P" || goto :failed
)

call :validate_cli || goto :failed

popd
echo.
echo [smoke] OK: generated and validated packages under %OUT_DIR%
echo [smoke] GUI binary: %XPKGUI%
exit /b 0

:failed
set "SMOKE_CODE=%ERRORLEVEL%"
popd >nul 2>nul
if "%SMOKE_CODE%"=="0" set "SMOKE_CODE=1"
exit /b %SMOKE_CODE%

:validate
set "PKG_NAME=%~1"
call :run "%XPKCON%" i "out\%PKG_NAME%.xpk" || exit /b 1
call :run "%XPKCON%" l "out\%PKG_NAME%.xpk" || exit /b 1
call :run "%XPKCON%" t "out\%PKG_NAME%.xpk" || exit /b 1
mkdir "extract\%PKG_NAME%" >nul 2>nul
call :run "%XPKCON%" e "out\%PKG_NAME%.xpk" -o"extract\%PKG_NAME%" || exit /b 1
exit /b 0

:validate_cli
echo [smoke] Validating xpkgui command-line entries...
echo [smoke] "%XPKGUI%" /verify "out\core.xpk"
call :run_gui "%XPKGUI%" /verify "out\core.xpk" || exit /b 1
echo [smoke] "%XPKGUI%" /properties "out\core.xpk"
call :run_gui "%XPKGUI%" /properties "out\core.xpk" || exit /b 1
mkdir "extract\cli_explicit" >nul 2>nul
echo [smoke] "%XPKGUI%" /extract "out\linux.xpk" "extract\cli_explicit"
call :run_gui "%XPKGUI%" /extract "out\linux.xpk" "extract\cli_explicit" || exit /b 1
echo [smoke] "%XPKGUI%" /extract-auto "out\index.xpk"
call :run_gui "%XPKGUI%" /extract-auto "out\index.xpk" || exit /b 1
echo [smoke] "%XPKGUI%" /extract-here "out\win32.xpk"
call :run_gui "%XPKGUI%" /extract-here "out\win32.xpk" || exit /b 1
if exist "input\xpack_archive.xpk" del /q "input\xpack_archive.xpk" || exit /b 1
echo [smoke] "%XPKGUI%" /shell-add-auto "input\alpha.txt" "input\sub\beta.txt"
call :run_gui "%XPKGUI%" /shell-add-auto "input\alpha.txt" "input\sub\beta.txt" || exit /b 1
if not exist "input\xpack_archive.xpk" (
	echo [ERROR] /shell-add-auto did not create input\xpack_archive.xpk
	exit /b 1
)
call :run "%XPKCON%" t "input\xpack_archive.xpk" || exit /b 1
echo [smoke] expect-fail "%XPKGUI%" /definitely-unknown-option
call :run_gui_expect_fail "%XPKGUI%" /definitely-unknown-option || exit /b 1
exit /b 0

:validate_package
echo [smoke] Validating staged package...
if not exist "%PKG_DIR%\xpkgui.exe" (
	echo [ERROR] staged package is missing xpkgui.exe.
	exit /b 1
)
if not exist "%PKG_DIR%\xpkgui_shext.dll" (
	echo [ERROR] staged package is missing xpkgui_shext.dll.
	exit /b 1
)
if not exist "%PKG_DIR%\install_x64.bat" (
	echo [ERROR] staged package is missing install_x64.bat.
	exit /b 1
)
if not exist "%PKG_DIR%\uninstall_x64.bat" (
	echo [ERROR] staged package is missing uninstall_x64.bat.
	exit /b 1
)
if not exist "%PKG_DIR%\docs\USER_GUIDE.md" (
	echo [ERROR] staged package is missing user guide.
	exit /b 1
)
if not exist "%PKG_DIR%\PACKAGE_MANIFEST.txt" (
	echo [ERROR] staged package is missing PACKAGE_MANIFEST.txt.
	exit /b 1
)
findstr /C:"xpkgui.exe" "%PKG_DIR%\PACKAGE_MANIFEST.txt" >nul || (
	echo [ERROR] staged package manifest is missing xpkgui.exe.
	exit /b 1
)
findstr /C:"xpkgui_shext.dll" "%PKG_DIR%\PACKAGE_MANIFEST.txt" >nul || (
	echo [ERROR] staged package manifest is missing xpkgui_shext.dll.
	exit /b 1
)
powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%verify_package_manifest.ps1" "%PKG_DIR%" || exit /b 1
exit /b 0

:static_checks
echo [smoke] Running static checks...
if not exist "%SCRIPT_DIR%install_x64.bat" (
	echo [ERROR] install_x64.bat is missing.
	exit /b 1
)
if not exist "%SCRIPT_DIR%uninstall_x64.bat" (
	echo [ERROR] uninstall_x64.bat is missing.
	exit /b 1
)
if not exist "%SCRIPT_DIR%check_release_x64.bat" (
	echo [ERROR] check_release_x64.bat is missing.
	exit /b 1
)
if not exist "%SCRIPT_DIR%package_x64.bat" (
	echo [ERROR] package_x64.bat is missing.
	exit /b 1
)
if not exist "%SCRIPT_DIR%write_package_manifest.ps1" (
	echo [ERROR] write_package_manifest.ps1 is missing.
	exit /b 1
)
if not exist "%SCRIPT_DIR%verify_package_manifest.ps1" (
	echo [ERROR] verify_package_manifest.ps1 is missing.
	exit /b 1
)
if not exist "%SCRIPT_DIR%show_spec_status.ps1" (
	echo [ERROR] show_spec_status.ps1 is missing.
	exit /b 1
)
if not exist "%SCRIPT_DIR%write_release_audit_report.ps1" (
	echo [ERROR] write_release_audit_report.ps1 is missing.
	exit /b 1
)
if not exist "%RUN_TIMEOUT%" (
	echo [ERROR] run_with_timeout.ps1 is missing.
	exit /b 1
)
findstr /C:"WaitForExit" "%RUN_TIMEOUT%" >nul || (
	echo [ERROR] run_with_timeout.ps1 is missing timeout waiting logic.
	exit /b 1
)
findstr /C:"Kill()" "%RUN_TIMEOUT%" >nul || (
	echo [ERROR] run_with_timeout.ps1 is missing timeout kill logic.
	exit /b 1
)
findstr /C:"PKG_ROOT" "%SCRIPT_DIR%package_x64.bat" >nul || (
	echo [ERROR] package_x64.bat does not stage the expected package directory.
	exit /b 1
)
findstr /C:"PKG_DIR" "%SCRIPT_DIR%package_x64.bat" >nul || (
	echo [ERROR] package_x64.bat does not define the package output directory.
	exit /b 1
)
findstr /C:"write_package_manifest.ps1" "%SCRIPT_DIR%package_x64.bat" >nul || (
	echo [ERROR] package_x64.bat does not generate PACKAGE_MANIFEST.txt.
	exit /b 1
)
findstr /C:"ErrorActionPreference = 'Stop'" "%SCRIPT_DIR%write_package_manifest.ps1" >nul || (
	echo [ERROR] write_package_manifest.ps1 does not fail on PowerShell errors.
	exit /b 1
)
findstr /C:"System.Security.Cryptography.SHA256" "%SCRIPT_DIR%write_package_manifest.ps1" >nul || (
	echo [ERROR] write_package_manifest.ps1 does not compute file hashes.
	exit /b 1
)
findstr /C:"Manifest hash mismatch" "%SCRIPT_DIR%verify_package_manifest.ps1" >nul || (
	echo [ERROR] verify_package_manifest.ps1 does not verify file hashes.
	exit /b 1
)
findstr /C:"Manifest size mismatch" "%SCRIPT_DIR%verify_package_manifest.ps1" >nul || (
	echo [ERROR] verify_package_manifest.ps1 does not verify file sizes.
	exit /b 1
)
findstr /C:"Remaining items:" "%SCRIPT_DIR%show_spec_status.ps1" >nul || (
	echo [ERROR] show_spec_status.ps1 does not summarize unfinished spec items.
	exit /b 1
)
findstr /C:"ReadAllLines" "%SCRIPT_DIR%show_spec_status.ps1" >nul || (
	echo [ERROR] show_spec_status.ps1 does not use deterministic file scanning.
	exit /b 1
)
findstr /C:"[regex]::IsMatch" "%SCRIPT_DIR%show_spec_status.ps1" >nul || (
	echo [ERROR] show_spec_status.ps1 does not use explicit checkbox matching.
	exit /b 1
)
findstr /C:"review tracked history before deleting" "%SCRIPT_DIR%write_release_audit_report.ps1" >nul || (
	echo [ERROR] write_release_audit_report.ps1 does not classify tracked unexpected files.
	exit /b 1
)
findstr /C:"/strict" "%SCRIPT_DIR%check_release_x64.bat" >nul || (
	echo [ERROR] release audit script is missing strict mode.
	exit /b 1
)
findstr /C:"RELEASE_MANIFEST.md" "%SCRIPT_DIR%check_release_x64.bat" >nul || (
	echo [ERROR] release audit script does not reference the release manifest.
	exit /b 1
)
findstr /C:"regsvr32" "%SCRIPT_DIR%install_x64.bat" >nul || (
	echo [ERROR] install_x64.bat does not reference regsvr32.
	exit /b 1
)
findstr /C:"xpkgui_shext.dll" "%SCRIPT_DIR%install_x64.bat" >nul || (
	echo [ERROR] install_x64.bat does not reference xpkgui_shext.dll.
	exit /b 1
)
findstr /C:"Exit code:" "%SCRIPT_DIR%install_x64.bat" >nul || (
	echo [ERROR] install_x64.bat is missing regsvr32 diagnostic output.
	exit /b 1
)
findstr /C:"Residual xpkgui registry entries" "%SCRIPT_DIR%uninstall_x64.bat" >nul || (
	echo [ERROR] uninstall_x64.bat is missing residual registry checks.
	exit /b 1
)
findstr /C:"#define XPKGUI_ARCHIVE_COLUMN_COUNT 10" "%SCRIPT_DIR%src\app.h" >nul || (
	echo [ERROR] XPKGUI_ARCHIVE_COLUMN_COUNT is not 10.
	exit /b 1
)
findstr /C:"ColumnCount" "%SCRIPT_DIR%src\common.c" >nul || (
	echo [ERROR] ColumnCount settings migration checks are missing.
	exit /b 1
)
findstr /C:"ShowGridLines" "%SCRIPT_DIR%src\common.c" >nul || (
	echo [ERROR] Show Grid Lines setting support is missing.
	exit /b 1
)
findstr /C:"FullRowSelect" "%SCRIPT_DIR%src\common.c" >nul || (
	echo [ERROR] Full Row Select setting support is missing.
	exit /b 1
)
findstr /C:"ID_VIEW_GRID_LINES" "%SCRIPT_DIR%src\resources\xpkgui.rc" >nul || (
	echo [ERROR] View menu is missing Show Grid Lines.
	exit /b 1
)
findstr /C:"ID_VIEW_FULL_ROW_SELECT" "%SCRIPT_DIR%src\resources\xpkgui.rc" >nul || (
	echo [ERROR] View menu is missing Full Row Select.
	exit /b 1
)
findstr /C:"ID_VIEW_COLUMNS" "%SCRIPT_DIR%src\resources\xpkgui.rc" >nul || (
	echo [ERROR] View menu is missing Columns dialog entry.
	exit /b 1
)
findstr /C:"Visible%%d" "%SCRIPT_DIR%src\common.c" >nul || (
	echo [ERROR] Column visibility persistence is missing.
	exit /b 1
)
findstr /C:"GuiVisibleColumnToLogical" "%SCRIPT_DIR%src\window.c" >nul || (
	echo [ERROR] Visible column sorting mapping is missing.
	exit /b 1
)
findstr /C:"GuiIsArchiveColumnVisible" "%SCRIPT_DIR%src\archive.c" >nul || (
	echo [ERROR] Visible column export/render strategy is missing.
	exit /b 1
)
findstr /C:"Edit Externally" "%SCRIPT_DIR%src\resources\xpkgui.rc" >nul || (
	echo [ERROR] External edit menu label is missing.
	exit /b 1
)
findstr /C:"Ratio" "%SCRIPT_DIR%src\archive.c" >nul || (
	echo [ERROR] Ratio list column support is missing.
	exit /b 1
)
findstr /C:"Physical Size" "%SCRIPT_DIR%src\archive.c" >nul || (
	echo [ERROR] Archive properties physical size summary is missing.
	exit /b 1
)
findstr /C:"Header + Meta + Table" "%SCRIPT_DIR%src\archive.c" >nul || (
	echo [ERROR] Archive properties header/table summary is missing.
	exit /b 1
)
findstr /C:"GuiArchivePropertyTotalsCallback" "%SCRIPT_DIR%src\archive.c" >nul || (
	echo [ERROR] Archive properties total size aggregation is missing.
	exit /b 1
)
findstr /C:"xpkEachMatch" "%SCRIPT_DIR%docs\API_COVERAGE.md" >nul || (
	echo [ERROR] API coverage document is missing xpkEachMatch assessment.
	exit /b 1
)
findstr /C:"xpkHash32" "%SCRIPT_DIR%docs\API_COVERAGE.md" >nul || (
	echo [ERROR] API coverage document is missing xpkHash32 assessment.
	exit /b 1
)
findstr /C:"Renumber Indexes" "%SCRIPT_DIR%docs\FEATURE_EVALUATION.md" >nul || (
	echo [ERROR] Feature evaluation is missing index renumbering decision.
	exit /b 1
)
findstr /C:"xpkPathSetTimes" "%SCRIPT_DIR%docs\FEATURE_EVALUATION.md" >nul || (
	echo [ERROR] Feature evaluation is missing timestamp editing decision.
	exit /b 1
)
findstr /C:"Background Archive Loading" "%SCRIPT_DIR%docs\FEATURE_EVALUATION.md" >nul || (
	echo [ERROR] Feature evaluation is missing large archive loading decision.
	exit /b 1
)
findstr /C:"Dark Mode" "%SCRIPT_DIR%docs\FEATURE_EVALUATION.md" >nul || (
	echo [ERROR] Feature evaluation is missing dark mode decision.
	exit /b 1
)
findstr /C:"High DPI Layout" "%SCRIPT_DIR%docs\FEATURE_EVALUATION.md" >nul || (
	echo [ERROR] Feature evaluation is missing high DPI layout decision.
	exit /b 1
)
findstr /C:"Error Categories" "%SCRIPT_DIR%docs\ERROR_HANDLING_REVIEW.md" >nul || (
	echo [ERROR] Error handling review document is missing categories.
	exit /b 1
)
findstr /C:"Temporary File Lifecycle" "%SCRIPT_DIR%docs\ERROR_HANDLING_REVIEW.md" >nul || (
	echo [ERROR] Error handling review document is missing temp lifecycle policy.
	exit /b 1
)
findstr /C:"Accepted Silent Returns" "%SCRIPT_DIR%docs\RETURN_FALSE_AUDIT.md" >nul || (
	echo [ERROR] Return FALSE audit is missing silent-return policy.
	exit /b 1
)
findstr /C:"Expected xpkgui Outputs" "%SCRIPT_DIR%docs\RELEASE_MANIFEST.md" >nul || (
	echo [ERROR] Release manifest is missing expected xpkgui outputs.
	exit /b 1
)
findstr /C:"Staged xpkgui Package" "%SCRIPT_DIR%docs\RELEASE_MANIFEST.md" >nul || (
	echo [ERROR] Release manifest is missing staged package details.
	exit /b 1
)
findstr /C:"Explorer Integration" "%SCRIPT_DIR%docs\MANUAL_REGRESSION.md" >nul || (
	echo [ERROR] Manual regression checklist is missing Explorer coverage.
	exit /b 1
)
findstr /C:"Baseline GUI Packages" "%SCRIPT_DIR%docs\MANUAL_REGRESSION.md" >nul || (
	echo [ERROR] Manual regression checklist is missing GUI package coverage.
	exit /b 1
)
findstr /C:"GuiShowSystemErrorDetail" "%SCRIPT_DIR%src\common.c" >nul || (
	echo [ERROR] Detailed system error diagnostics are missing.
	exit /b 1
)
findstr /C:"FILE_ATTRIBUTE_READONLY" "%SCRIPT_DIR%src\archive.c" >nul || (
	echo [ERROR] Read-only archive save diagnostics are missing.
	exit /b 1
)
findstr /C:"GuiCleanupStaleTempRoots" "%SCRIPT_DIR%src\main.c" >nul || (
	echo [ERROR] Stale temporary root cleanup is not called at startup.
	exit /b 1
)
findstr /C:"XPKGUI_TEMP_ROOT_MAX_AGE_100NS" "%SCRIPT_DIR%src\common.c" >nul || (
	echo [ERROR] Stale temporary root age policy is missing.
	exit /b 1
)
findstr /C:"write-back fails" "%SCRIPT_DIR%docs\ERROR_HANDLING_REVIEW.md" >nul || (
	echo [ERROR] External edit write-back recovery policy is missing.
	exit /b 1
)
findstr /C:"directories younger than 7 days are kept" "%SCRIPT_DIR%docs\ERROR_HANDLING_REVIEW.md" >nul || (
	echo [ERROR] Stale temp cleanup retention policy is missing.
	exit /b 1
)
findstr /C:"Changing Pack Type changes entry semantics" "%SCRIPT_DIR%src\resources\xpkgui.rc" >nul || (
	echo [ERROR] Archive settings pack type warning text is missing.
	exit /b 1
)
findstr /C:"XPK_VOLUME_MIN" "%SCRIPT_DIR%src\dialogs.c" >nul || (
	echo [ERROR] Archive settings volume minimum validation is missing.
	exit /b 1
)
findstr /C:"packTypeChanged || solidChanged || volumeChanged" "%SCRIPT_DIR%src\archive.c" >nul || (
	echo [ERROR] Archive settings risky save confirmation is missing.
	exit /b 1
)
findstr /C:"canExtractToolbar" "%SCRIPT_DIR%src\window.c" >nul || (
	echo [ERROR] Toolbar enable state refinement is missing.
	exit /b 1
)
findstr /C:"GUI_TASK_RESULT_CANCELLED && !GuiIsSmokeMode" "%SCRIPT_DIR%src\archive.c" >nul || (
	echo [ERROR] Task cancellation result explanation is missing.
	exit /b 1
)
findstr /C:"exitAfterCreate && !GuiIsSmokeMode" "%SCRIPT_DIR%src\archive.c" >nul || (
	echo [ERROR] Shell add auto success dialog is not suppressed in smoke mode.
	exit /b 1
)
exit /b 0

:write_release_audit_report
powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%write_release_audit_report.ps1" "%RELEASE_AUDIT_REPORT%" || exit /b 1
if not exist "%RELEASE_AUDIT_REPORT%" (
	echo [ERROR] release audit report was not generated.
	exit /b 1
)
findstr /C:"Unexpected items:" "%RELEASE_AUDIT_REPORT%" >nul || (
	echo [ERROR] release audit report is missing summary.
	exit /b 1
)
exit /b 0

:run_gui
powershell -NoProfile -ExecutionPolicy Bypass -File "%RUN_TIMEOUT%" 30 %*
if errorlevel 1 (
	echo [ERROR] GUI command failed or timed out: %*
	exit /b 1
)
exit /b 0

:run_gui_expect_fail
powershell -NoProfile -ExecutionPolicy Bypass -File "%RUN_TIMEOUT%" 30 %*
if errorlevel 1 (
	exit /b 0
)
echo [ERROR] Command unexpectedly succeeded: %*
exit /b 1

:run
echo [smoke] %*
%*
if errorlevel 1 (
	echo [ERROR] Command failed: %*
	exit /b 1
)
exit /b 0

:run_expect_fail
echo [smoke] expect-fail %*
%*
if errorlevel 1 (
	exit /b 0
)
echo [ERROR] Command unexpectedly succeeded: %*
exit /b 1
