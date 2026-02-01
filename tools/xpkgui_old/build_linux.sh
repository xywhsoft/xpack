#!/bin/bash
set -e

echo "========================================"
echo "Building xpkgui (Linux, static)"
echo "========================================"
echo

OUTPUT_DIR="../../release/linux"
OUTPUT="$OUTPUT_DIR/xpkgui"
mkdir -p "$OUTPUT_DIR"

CC=${CC:-gcc}
CFLAGS="-I../../lib -I../../src -DXPK_BUILD_LIB -DZSTD_NO_INTRINSICS -DZ7_ST -DXPK_WITH_SQLITE -O2 -Wall"
CFLAGS+=" $(pkg-config --cflags gtk+-3.0 2>/dev/null || echo '')"
LDFLAGS="-lm -lpthread"
LDFLAGS+=" $(pkg-config --libs gtk+-3.0 2>/dev/null || echo '')"

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
	xpkgui.c
)

echo "Compiler: $CC"
echo "Output: $OUTPUT"
echo

$CC -o "$OUTPUT" $CFLAGS "${SOURCES[@]}" $LDFLAGS

if [ $? -eq 0 ]; then
	echo
	echo "========================================"
	echo "Build successful (static)!"
	echo "Output: $OUTPUT"
	echo "Note: xpkgui is standalone."
	echo "========================================"
	chmod +x "$OUTPUT"
else
	echo
	echo "Build failed!"
	exit 1
fi
