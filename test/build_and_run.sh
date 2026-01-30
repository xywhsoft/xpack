#!/bin/bash

# ============================================================
# xPack Ver7 - 测试编译和运行脚本 (Linux)
# ============================================================

SRC_DIR="../src"
TEST_DIR="."
LIB_DIR="../lib"
OUTPUT_DIR="../release/x64"
XRT_DIR="${LIB_DIR}/xrt"

# 编译参数
CFLAGS="-m64 -I.. -I../src -I${LIB_DIR} -I${LIB_DIR}/zstd -I${LIB_DIR}/lzma -DZ7_ST -DDEBUG_TRACE"
LDFLAGS="-lpthread -ldl"
OPTFLAGS="-O2 -s -ffunction-sections -fdata-sections -Wl,--gc-sections"

# 源文件列表
XPACK_SRC="${SRC_DIR}/xpack.c ${SRC_DIR}/xpack_compress.c ${SRC_DIR}/xpack_core.c ${SRC_DIR}/xpack_index.c ${SRC_DIR}/xpack_ldb.c ${SRC_DIR}/xpack_path.c ${SRC_DIR}/xpack_util.c"
XRT_SRC="${XRT_DIR}/xrt.c"
LZ4_SRC="${LIB_DIR}/lz4/lz4.c ${LIB_DIR}/lz4/lz4hc.c"
ZSTD_SRC="${LIB_DIR}/zstd/zstd.c"
LZMA_SRC="${LIB_DIR}/lzma/Alloc.c ${LIB_DIR}/lzma/CpuArch.c ${LIB_DIR}/lzma/LzFind.c ${LIB_DIR}/lzma/LzmaDec.c ${LIB_DIR}/lzma/LzmaEnc.c ${LIB_DIR}/lzma/Lzma2Dec.c ${LIB_DIR}/lzma/Lzma2Enc.c"

echo "============================================================"
echo "  xPack Ver7 - Test Compilation and Execution (Linux)"
echo "============================================================"
echo ""

# 创建输出目录
mkdir -p "${OUTPUT_DIR}"

# 清理旧的测试文件
rm -f ${OUTPUT_DIR}/test_*.xpk 2>/dev/null

# 编译测试运行器（统一运行所有测试）
echo "[1/2] Compiling test runner..."
gcc ${CFLAGS} ${OPTFLAGS} test_framework.c test_runner.c ${XPACK_SRC} ${XRT_SRC} ${LZ4_SRC} ${ZSTD_SRC} ${LZMA_SRC} ${LDFLAGS} -o ${OUTPUT_DIR}/test_runner
if [ $? -ne 0 ]; then
    echo "  ERROR: Failed to compile test runner"
    exit 1
fi

echo "[2/2] Running test suite..."
cd ${OUTPUT_DIR}
./test_runner
RESULT=$?
cd ${TEST_DIR}

echo ""
if [ $RESULT -eq 0 ]; then
    echo "============================================================"
    echo "  All tests passed!"
    echo "============================================================"
    echo ""
    exit 0
else
    echo "============================================================"
    echo "  Some tests failed. See output above for details."
    echo "============================================================"
    echo ""
    exit 1
fi
