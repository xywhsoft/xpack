@echo off
cd /d "%~dp0"

call "%~dp0build_CLANG_ENV_x64.bat" || exit /b 1

if not exist "release\x64" mkdir "release\x64"

"%XPACK_CLANG_CL%" /nologo /TC /utf-8 /std:c11 /W3 /O2 /DXPACK_SELF_TEST_MAIN /DZ7_ST /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS /D_WINSOCK_DEPRECATED_NO_WARNINGS ^
	/DXRT_NO_COROUTINE /DXRT_NO_NETWORK /DXRT_NO_XURL /DXRT_NO_HTTP_UTIL /DXRT_NO_XCODEC /DXRT_NO_CRYPTO /DXRT_NO_NETTLS /DXRT_NO_XHTTP /DXRT_NO_XHTTPD /DXRT_NO_XWS /DXRT_NO_XID /DXRT_NO_BUFFER /DXRT_NO_STACK /DXRT_NO_REGEX /DXRT_NO_VALUE /DXRT_NO_JSON /DXRT_NO_TEMPLATE ^
	xpack.c lib\lz4\lz4.c lib\lz4\lz4hc.c lib\zstd\zstd.c lib\lzma\Alloc.c lib\lzma\CpuArch.c lib\lzma\LzFind.c lib\lzma\LzmaDec.c lib\lzma\LzmaEnc.c lib\lzma\Lzma2Dec.c lib\lzma\Lzma2Enc.c ^
	/I. /Ilib\lz4 /Ilib\zstd /Ilib\lzma /link /OUT:release\x64\xpack_test.exe Ws2_32.lib IPHLPAPI.lib || exit /b 1

echo.
echo Build successful: release\x64\xpack_test.exe
