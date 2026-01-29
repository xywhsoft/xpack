@echo off
REM xPack Ver7 Build Script
REM 
REM ZSTD: Original single-file version from facebook/zstd
REM   - zstd.c: Full version (compress + decompress + dict builder)
REM   - zstddeclib.c: Decoder only (for read-only builds)

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
    lib/zstd/zstd.c ^
    test/test_main.c ^
    -Ilib -Ilib/zstd -Isrc ^
    -O2 -s -ffunction-sections -fdata-sections -Wl,--gc-sections ^
    -lws2_32 -lIPHLPAPI ^
    -o release/x64/test.exe

pause
