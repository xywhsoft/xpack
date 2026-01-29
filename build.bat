@echo off
REM xPack Ver7 Build Script
REM 
REM Compression Libraries:
REM   - LZ4/LZ4HC: Fast compression (levels 1-5)
REM   - ZSTD: Balanced compression (levels 6-14)
REM   - LZMA2: Maximum compression (level 15)

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
    lib/lzma/Alloc.c ^
    lib/lzma/CpuArch.c ^
    lib/lzma/LzFind.c ^
    lib/lzma/LzmaDec.c ^
    lib/lzma/LzmaEnc.c ^
    lib/lzma/Lzma2Dec.c ^
    lib/lzma/Lzma2Enc.c ^
    test/test_main.c ^
    -DZ7_ST ^
    -Ilib -Ilib/zstd -Ilib/lzma -Isrc ^
    -O2 -s -ffunction-sections -fdata-sections -Wl,--gc-sections ^
    -lws2_32 -lIPHLPAPI ^
    -o release/x64/test.exe

pause
