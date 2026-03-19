#!/bin/bash
set -e

echo "========================================"
echo "Building xpkcon (Linux, static)"
echo "========================================"
echo

OUTPUT_DIR="../../release/linux"
OUTPUT="$OUTPUT_DIR/xpkcon"
mkdir -p "$OUTPUT_DIR"

CC=${CC:-gcc}
CFLAGS="-I../../lib -I../../src -DXPK_BUILD_LIB -DZSTD_NO_INTRINSICS -DZ7_ST -DXPK_WITH_SQLITE -O2 -Wall"
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
	xpkcon.c
)

echo "Compiler: $CC"
echo "Output: $OUTPUT"
echo

$CC -o "$OUTPUT" $CFLAGS "${SOURCES[@]}" -lm -lpthread

if [ $? -eq 0 ]; then
	echo
	echo "========================================"
	echo "Build successful (static)!"
	echo "Output: $OUTPUT"
	echo "Note: xpkcon is standalone, no external libraries required."
	echo "========================================"
	chmod +x "$OUTPUT"
else
	echo
	echo "Build failed!"
	exit 1
fi
