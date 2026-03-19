#!/bin/bash
echo "Building xpkcon Linux (GCC, static)..."
echo

OUTPUT_DIR="../../release/linux"
OUTPUT="$OUTPUT_DIR/xpkcon"

mkdir -p "$OUTPUT_DIR"

gcc -o "$OUTPUT" \
	-I../../lib -I../../src \
	../../lib/xrt/xrt.c \
	../../lib/lz4/lz4.c \
	../../lib/lz4/lz4hc.c \
	../../lib/lzma/Alloc.c \
	../../lib/lzma/CpuArch.c \
	../../lib/lzma/LzFind.c \
	../../lib/lzma/LzmaDec.c \
	../../lib/lzma/LzmaEnc.c \
	../../lib/lzma/Lzma2Dec.c ^
	../../lib/lzma/Lzma2Enc.c ^
	../../lib/zstd/zstd.c ^
	../../src/xpack.c ^
	../../src/xpack_core.c ^
	../../src/xpack_index.c ^
	../../src/xpack_path.c ^
	../../src/xpack_util.c ^
	../../src/xpack_compress.c ^
	../../src/xpack_ldb.c ^
	../../src/xpack_volume.c \
	xpkcon.c \
	-DZ7_ST \
	-DXPK_BUILD_LIB \
	-lpthread -ldl -lm

if [ $? -eq 0 ]; then
	echo
	echo "Build successful!"
	echo "Output: $OUTPUT"
	chmod +x "$OUTPUT"
else
	echo
	echo "Build failed!"
	exit 1
fi
