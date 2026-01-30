# xPack Ver7 - Test Compilation and Execution (PowerShell)

$ErrorActionPreference = "Stop"

$SRC_DIR = "..\src"
$TEST_DIR = "."
$LIB_DIR = "..\lib"
$OUTPUT_DIR = "..\release\x64"
$XRT_DIR = "$LIB_DIR\xrt"

$CFLAGS = "-m64 -I.. -I..\src -I$LIB_DIR -I$LIB_DIR\zstd -I$LIB_DIR\lzma -DZ7_ST -DDEBUG_TRACE"
$LDFLAGS = "-lws2_32 -lIPHLPAPI"
$OPTFLAGS = "-O2 -s -ffunction-sections -fdata-sections -Wl,--gc-sections"

$XPACK_SRC = "$SRC_DIR\xpack.c", "$SRC_DIR\xpack_compress.c", "$SRC_DIR\xpack_core.c", "$SRC_DIR\xpack_index.c", "$SRC_DIR\xpack_ldb.c", "$SRC_DIR\xpack_path.c", "$SRC_DIR\xpack_util.c"
$XRT_SRC = "$XRT_DIR\xrt.c"
$LZ4_SRC = "$LIB_DIR\lz4\lz4.c", "$LIB_DIR\lz4\lz4hc.c"
$ZSTD_SRC = "$LIB_DIR\zstd\zstd.c"
$LZMA_SRC = "$LIB_DIR\lzma\Alloc.c", "$LIB_DIR\lzma\CpuArch.c", "$LIB_DIR\lzma\LzFind.c", "$LIB_DIR\lzma\LzmaDec.c", "$LIB_DIR\lzma\LzmaEnc.c", "$LIB_DIR\lzma\Lzma2Dec.c", "$LIB_DIR\lzma\Lzma2Enc.c"

Write-Host "============================================================"
Write-Host "  xPack Ver7 - Test Compilation and Execution"
Write-Host "============================================================"
Write-Host ""

if (-not (Test-Path $OUTPUT_DIR)) {
    New-Item -ItemType Directory -Path $OUTPUT_DIR | Out-Null
}

Remove-Item -Path "$OUTPUT_DIR\test_*.xpk" -ErrorAction SilentlyContinue | Out-Null

Write-Host "[1/2] Compiling test runner..."
$sourceFiles = @("test_framework.c", "test_runner.c") + $XPACK_SRC + $XRT_SRC + $LZ4_SRC + $ZSTD_SRC + $LZMA_SRC

$argsList = @(
    "-m64"
    "-I.."
    "-I..\src"
    "-I$LIB_DIR"
    "-I$LIB_DIR\zstd"
    "-I$LIB_DIR\lzma"
    "-DZ7_ST"
    "-DDEBUG_TRACE"
    "-O2"
    "-s"
    "-ffunction-sections"
    "-fdata-sections"
    "-Wl,--gc-sections"
) + $sourceFiles + @(
    "-lws2_32"
    "-lIPHLPAPI"
    "-o"
    "$OUTPUT_DIR\test_runner.exe"
)

$logfile = "$OUTPUT_DIR\compile_errors.txt"

$output = cmd /c "gcc -m64 -I.. -I..\src -I..\lib -I..\lib\zstd -I..\lib\lzma -DZ7_ST -DDEBUG_TRACE -O2 -s -ffunction-sections -fdata-sections -Wl^,--gc-sections test_framework.c test_runner.c ..\src\xpack.c ..\src\xpack_compress.c ..\src\xpack_core.c ..\src\xpack_index.c ..\src\xpack_ldb.c ..\src\xpack_path.c ..\src\xpack_util.c ..\lib\xrt\xrt.c ..\lib\lz4\lz4.c ..\lib\lz4\lz4hc.c ..\lib\zstd\zstd.c ..\lib\lzma\Alloc.c ..\lib\lzma\CpuArch.c ..\lib\lzma\LzFind.c ..\lib\lzma\LzmaDec.c ..\lib\lzma\LzmaEnc.c ..\lib\lzma\Lzma2Dec.c ..\lib\lzma\Lzma2Enc.c -lws2_32 -lIPHLPAPI -o ..\release\x64\test_runner.exe" 2>&1 | Out-String

if ($LASTEXITCODE -ne 0) {
    Write-Host "  ERROR: Failed to compile test runner"
    Write-Host "  Check $logfile for details"
    $output | Out-File -FilePath $logfile -Encoding UTF8
    exit 1
}

Write-Host "[2/2] Running test suite..."
Push-Location $OUTPUT_DIR
try {
    & ".\test_runner.exe"
    $result = $LASTEXITCODE
} finally {
    Pop-Location
}

Write-Host ""
if ($result -eq 0) {
    Write-Host "============================================================"
    Write-Host "  All tests passed!"
    Write-Host "============================================================"
    Write-Host ""
    exit 0
} else {
    Write-Host "============================================================"
    Write-Host "  Some tests failed. See output above for details."
    Write-Host "============================================================"
    Write-Host ""
    exit 1
}
