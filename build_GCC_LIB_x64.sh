#!/bin/bash

gcc -m64 -c \
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
	-Ilib -Ilib/zstd -Ilib/lzma -Isrc \
	-O2 -ffunction-sections -fdata-sections

ar rcs release/x64/libxpack.a *.o

rm -f *.o

echo ""
