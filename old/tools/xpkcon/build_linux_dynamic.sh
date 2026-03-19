#!/bin/bash
set -e

echo "========================================"
echo "Building xpkcon (Linux, dynamic)"
echo "========================================"
echo

OUTPUT_DIR="../../release/linux"
OUTPUT="$OUTPUT_DIR/xpkcon"
LIB_OUTPUT="$OUTPUT_DIR/libxpack.so"
mkdir -p "$OUTPUT_DIR"

CC=${CC:-gcc}
CFLAGS="-I../../lib -I../../src -DXPK_BUILD_DLL -DZSTD_NO_INTRINSICS -DZ7_ST -O2 -Wall -fPIC"
LDFLAGS="-shared -Wl,-soname,libxpack.so"

SOURCES=(
	../../lib/xrt/xrt.c
	../../lib/lz4/lz4.c
	../../lib/lz4/lz4hc.c
	../../lib/lzma/Alloc.c
	../../lib/lzma/CpuArch.c
	../../lib/lzma/LzFind.c
	../../lib/lzma/LzmaDec.c
	../../lib/lzma/LzmaEnc.c
	../../lib/lzma/Lzma2Dec.c
	../../lib/lzma/Lzma2Enc.c
	../../lib/zstd/zstd.c
	../../src/xpack.c
	../../src/xpack_core.c
	../../src/xpack_index.c
	../../src/xpack_path.c
	../../src/xpack_util.c
	../../src/xpack_compress.c
	../../src/xpack_ldb.c
	../../src/xpack_volume.c
)

echo "Building libxpack.so..."
$CC -o "$LIB_OUTPUT" $CFLAGS $LDFLAGS "${SOURCES[@]}" -lm -lpthread

if [ $? -eq 0 ]; then
	echo "Library built successfully: $LIB_OUTPUT"
else
	echo "Building library failed!"
	exit 1
fi

echo
echo "Building xpkcon..."
$CC -o "$OUTPUT" -I../../lib -I../../src -I"$OUTPUT_DIR" \
	-DXPK_BUILD_DLL -DZSTD_NO_INTRINSICS -DZ7_ST -O2 -Wall \
	xpkcon.c -L"$OUTPUT_DIR" -lxpack -Wl,-rpath,"$OUTPUT_DIR"

if [ $? -eq 0 ]; then
	echo
	echo "========================================"
	echo "Build successful (dynamic)!"
	echo "Output: $OUTPUT"
	echo "Library: $LIB_OUTPUT"
	echo "Note: xpkcon requires libxpack.so to run."
	echo "========================================"
	chmod +x "$OUTPUT"
else
	echo
	echo "Build failed!"
	exit 1
fi
