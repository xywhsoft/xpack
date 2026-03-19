#!/bin/sh

set -e

mkdir -p release/x64

XRT_TRIM="\
-DXRT_NO_COROUTINE \
-DXRT_NO_NETWORK \
-DXRT_NO_XURL \
-DXRT_NO_HTTP_UTIL \
-DXRT_NO_XCODEC \
-DXRT_NO_CRYPTO \
-DXRT_NO_NETTLS \
-DXRT_NO_XHTTP \
-DXRT_NO_XHTTPD \
-DXRT_NO_XWS \
-DXRT_NO_XID \
-DXRT_NO_BUFFER \
-DXRT_NO_STACK \
-DXRT_NO_REGEX \
-DXRT_NO_VALUE \
-DXRT_NO_JSON \
-DXRT_NO_TEMPLATE"

gcc -shared -m64 \
    xpack.c \
    lib/lz4/lz4.c \
    lib/lz4/lz4hc.c \
    lib/zstd/zstd.c \
    lib/lzma/Alloc.c \
    lib/lzma/CpuArch.c \
    lib/lzma/LzFind.c \
    lib/lzma/LzmaDec.c \
    lib/lzma/LzmaEnc.c \
    lib/lzma/Lzma2Dec.c \
    lib/lzma/Lzma2Enc.c \
    -Ilib -Ilib/lz4 -Ilib/zstd -Ilib/lzma \
    -DXPACK_NO_MAIN -DZ7_ST $XRT_TRIM \
    -O2 -s -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections \
    -o release/x64/xpack.so

echo
echo "Build successful: release/x64/xpack.so"
