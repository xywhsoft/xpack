#!/bin/bash

gcc -m64 \
	test/test_main.c \
	src/xpack.c \
	src/xpack_compress.c \
	src/xpack_core.c \
	src/xpack_index.c \
	src/xpack_ldb.c \
	src/xpack_path.c \
	src/xpack_util.c \
	lib/xrt/xrt.c \
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
	-DZ7_ST \
	-DDEBUG_TRACE \
	-Ilib -Ilib/zstd -Ilib/lzma -Isrc \
	-O2 -s -ffunction-sections -fdata-sections -Wl,--gc-sections \
	-lpthread -lm \
	-o release/x64/test

echo ""
cd release/x64
./test
