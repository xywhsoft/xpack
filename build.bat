@echo off
REM xPack Ver7 Build Script - Minimal ZSTD + rapidhash
REM 
REM ZSTD Optimizations:
REM   -DZSTD_DISABLE_ASM    : Disable assembly (portable)
REM   -DZSTD_NO_TRACE       : Disable trace
REM   No ZSTD_MULTITHREAD   : Single-threaded (removes pool.c, threading.c)
REM   xxhash replaced       : Using rapidhash compatibility layer (7KB vs 248KB)

gcc ^
    src/xpack.c ^
    src/xpack_compress.c ^
    src/xpack_core.c ^
    src/xpack_index.c ^
    src/xpack_path.c ^
    src/xpack_ldb.c ^
    src/xpack_util.c ^
    lib/xrt/xrt.c ^
    lib/lz4/lz4.c ^
    lib/lz4/lz4hc.c ^
    lib/zstd/common/debug.c ^
    lib/zstd/common/entropy_common.c ^
    lib/zstd/common/error_private.c ^
    lib/zstd/common/fse_decompress.c ^
    lib/zstd/common/zstd_common.c ^
    lib/zstd/compress/fse_compress.c ^
    lib/zstd/compress/hist.c ^
    lib/zstd/compress/huf_compress.c ^
    lib/zstd/compress/zstd_compress.c ^
    lib/zstd/compress/zstd_compress_literals.c ^
    lib/zstd/compress/zstd_compress_sequences.c ^
    lib/zstd/compress/zstd_compress_superblock.c ^
    lib/zstd/compress/zstd_double_fast.c ^
    lib/zstd/compress/zstd_fast.c ^
    lib/zstd/compress/zstd_lazy.c ^
    lib/zstd/compress/zstd_ldm.c ^
    lib/zstd/compress/zstd_opt.c ^
    lib/zstd/compress/zstd_preSplit.c ^
    lib/zstd/decompress/huf_decompress.c ^
    lib/zstd/decompress/zstd_ddict.c ^
    lib/zstd/decompress/zstd_decompress.c ^
    lib/zstd/decompress/zstd_decompress_block.c ^
    test/test_main.c ^
    -Ilib -Ilib/zstd -Isrc ^
    -DZSTD_DISABLE_ASM -DZSTD_NO_TRACE ^
    -O2 -s -ffunction-sections -fdata-sections -Wl,--gc-sections ^
    -lws2_32 -lIPHLPAPI ^
    -o release/x64/test.exe
