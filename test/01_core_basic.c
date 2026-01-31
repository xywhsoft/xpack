/*
 * xPack Ver7 - 核心基本操作测试
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <xrt/xrt.h>
#include "../src/xpack.h"

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) void test_##name(void)
#define RUN_TEST(name) do { \
    printf("  Testing %s... ", #name); \
    test_##name(); \
    printf("PASSED\n"); \
    tests_passed++; \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAILED\n"); \
        printf("    Assertion failed: %s\n", #cond); \
        printf("    At line %d\n", __LINE__); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_NE(a, b) ASSERT((a) != (b))
#define ASSERT_NULL(a) ASSERT((a) == NULL)
#define ASSERT_NOT_NULL(a) ASSERT((a) != NULL)
#define ASSERT_GT(a, b) ASSERT((a) > (b))
#define ASSERT_LT(a, b) ASSERT((a) < (b))

TEST(core_initialization) {
    xpkObject xpk = xpkOpen("test_01_init.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkType(xpk), XPK_TYPE_CORE);
    ASSERT_EQ(xpkCount(xpk), 0);

    xpkClose(xpk);
}

TEST(core_create_empty_package) {
    xpkObject xpk = xpkOpen("test_01_empty.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkCount(xpk), 0);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_01_empty.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 0);
    xpkClose(xpk);
}

TEST(core_add_file) {
    xpkObject xpk = xpkOpen("test_01_add_file.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    char testData[256];
    memset(testData, 'A', sizeof(testData));
    strcpy(testData, "Test file content");

    char srcPath[256];
    sprintf(srcPath, "test_src_01.txt");
    FILE* fp = fopen(srcPath, "wb");
    fwrite(testData, 1, sizeof(testData), fp);
    fclose(fp);

    uint32_t pos = xpkAppendFile(xpk, srcPath, 6);
    ASSERT_EQ(pos, 0);
    ASSERT_EQ(xpkCount(xpk), 1);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_01_add_file.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 1);
    xpkClose(xpk);

    xrtFileDelete(srcPath);
}

TEST(core_add_directory) {
    xpkObject xpk = xpkOpen("test_01_add_dir.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    char dirPath[256];
    sprintf(dirPath, "test_dir_01");
    xrtMkDir(dirPath);

    char filePath[256];
    sprintf(filePath, "%s/file1.txt", dirPath);
    FILE* fp = fopen(filePath, "wb");
    fwrite("Content 1", 1, 9, fp);
    fclose(fp);

    sprintf(filePath, "%s/file2.txt", dirPath);
    fp = fopen(filePath, "wb");
    fwrite("Content 2", 1, 9, fp);
    fclose(fp);

    uint32_t pos = xpkAppendFile(xpk, dirPath, 6);
    ASSERT_NE(pos, UINT32_MAX);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_01_add_dir.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_GT(xpkCount(xpk), 0);
    xpkClose(xpk);

    xrtRmDirRecursive(dirPath);
}

TEST(core_add_data) {
    xpkObject xpk = xpkOpen("test_01_add_data.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    char data[] = "Test data from memory";
    uint32_t pos = xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
    ASSERT_EQ(pos, 0);
    ASSERT_EQ(xpkCount(xpk), 1);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_01_add_data.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 1);

    uint32_t outSize = 0;
    void* extracted = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(extracted);
    ASSERT_EQ(outSize, strlen(data));
    ASSERT_EQ(memcmp(extracted, data, outSize), 0);
    xpkFree(extracted);

    xpkClose(xpk);
}

TEST(core_save_load) {
    xpkObject xpk = xpkOpen("test_01_save_load.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    xpkAppendData(xpk, "First data", 10, 6);
    xpkAppendData(xpk, "Second data", 11, 6);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_01_save_load.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 2);

    uint32_t outSize = 0;
    void* data = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(outSize, 10);
    ASSERT_EQ(memcmp(data, "First data", 10), 0);
    xpkFree(data);

    data = xpkExtractData(xpk, 1, &outSize);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(outSize, 11);
    ASSERT_EQ(memcmp(data, "Second data", 11), 0);
    xpkFree(data);

    xpkClose(xpk);
}

TEST(core_compression_lz4) {
    xpkObject xpk = xpkOpen("test_01_lz4.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    char testData[4096];
    memset(testData, 'X', sizeof(testData));

    xpkAppendData(xpk, testData, sizeof(testData), 1);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_01_lz4.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    uint32_t packedSize = xpkInfoPacked(xpk, 0);
    ASSERT_GT(packedSize, 0);
    ASSERT_LT(packedSize, sizeof(testData));

    uint32_t outSize = 0;
    void* extracted = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(extracted);
    ASSERT_EQ(outSize, sizeof(testData));
    ASSERT_EQ(memcmp(extracted, testData, outSize), 0);
    xpkFree(extracted);

    xpkClose(xpk);
}

TEST(core_compression_zstd) {
    xpkObject xpk = xpkOpen("test_01_zstd.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    char testData[4096];
    for (int i = 0; i < 4096; i++) {
        testData[i] = (char)(i % 256);
    }

    xpkAppendData(xpk, testData, sizeof(testData), 6);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_01_zstd.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    uint32_t packedSize = xpkInfoPacked(xpk, 0);
    ASSERT_GT(packedSize, 0);
    ASSERT_LT(packedSize, sizeof(testData));

    uint32_t outSize = 0;
    void* extracted = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(extracted);
    ASSERT_EQ(outSize, sizeof(testData));
    ASSERT_EQ(memcmp(extracted, testData, outSize), 0);
    xpkFree(extracted);

    xpkClose(xpk);
}

TEST(core_compression_lzma2) {
    xpkObject xpk = xpkOpen("test_01_lzma2.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    char testData[8192];
    for (int i = 0; i < 8192; i++) {
        testData[i] = (char)(i % 16);
    }

    xpkAppendData(xpk, testData, sizeof(testData), 11);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_01_lzma2.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    uint32_t packedSize = xpkInfoPacked(xpk, 0);
    ASSERT_GT(packedSize, 0);
    ASSERT_LT(packedSize, sizeof(testData));

    uint32_t outSize = 0;
    void* extracted = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(extracted);
    ASSERT_EQ(outSize, sizeof(testData));
    ASSERT_EQ(memcmp(extracted, testData, outSize), 0);
    xpkFree(extracted);

    xpkClose(xpk);
}

TEST(core_compression_all_algorithms) {
    int levels[] = {1, 6, 11};
    const char* names[] = {"LZ4", "ZSTD", "LZMA2"};

    for (int i = 0; i < 3; i++) {
        char filename[64];
        sprintf(filename, "test_01_algo_%s.xpk", names[i]);

        xpkObject xpk = xpkOpen(filename, 0, 0);
        ASSERT_NOT_NULL(xpk);

        char testData[2048];
        memset(testData, 'A' + i, sizeof(testData));

        xpkAppendData(xpk, testData, sizeof(testData), levels[i]);
        ASSERT_EQ(xpkSave(xpk), 0);
        xpkClose(xpk);

        xpk = xpkOpen(filename, 0, 1);
        ASSERT_NOT_NULL(xpk);

        uint32_t packedSize = xpkInfoPacked(xpk, 0);
        ASSERT_GT(packedSize, 0);

        uint32_t outSize = 0;
        void* extracted = xpkExtractData(xpk, 0, &outSize);
        ASSERT_NOT_NULL(extracted);
        ASSERT_EQ(outSize, sizeof(testData));
        ASSERT_EQ(memcmp(extracted, testData, outSize), 0);
        xpkFree(extracted);

        xpkClose(xpk);
    }
}

TEST(core_package_properties) {
    xpkObject xpk = xpkOpen("test_01_props.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkType(xpk), XPK_TYPE_CORE);
    ASSERT_EQ(xpkCount(xpk), 0);
    ASSERT_EQ(xpkSolidMode(xpk), 0);

    xpkAppendData(xpk, "Test", 4, 6);

    ASSERT_EQ(xpkCount(xpk), 1);
    ASSERT_EQ(xpkInfoLevel(xpk, 0), 6);
    ASSERT_EQ(xpkInfoSize(xpk, 0), 4);
    ASSERT_GT(xpkInfoPacked(xpk, 0), 0);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);
}

TEST(core_file_count) {
    xpkObject xpk = xpkOpen("test_01_count.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkCount(xpk), 0);

    for (int i = 0; i < 10; i++) {
        xpkAppendData(xpk, "Data", 4, 6);
        ASSERT_EQ(xpkCount(xpk), i + 1);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_01_count.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 10);
    xpkClose(xpk);
}

TEST(core_traverse_files) {
    xpkObject xpk = xpkOpen("test_01_traverse.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 5; i++) {
        char data[32];
        sprintf(data, "File %d", i);
        xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
    }

    ASSERT_EQ(xpkCount(xpk), 5);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_01_traverse.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    int count = 0;
    for (uint32_t i = 0; i < xpkCount(xpk); i++) {
        uint32_t size = xpkInfoSize(xpk, i);
        ASSERT_GT(size, 0);
        count++;
    }

    ASSERT_EQ(count, 5);
    xpkClose(xpk);
}

TEST(core_extract_file) {
    xpkObject xpk = xpkOpen("test_01_extract.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    char testData[256];
    memset(testData, 'E', sizeof(testData));

    char srcPath[256];
    sprintf(srcPath, "test_src_extract.txt");
    FILE* fp = fopen(srcPath, "wb");
    fwrite(testData, 1, sizeof(testData), fp);
    fclose(fp);

    xpkAppendFile(xpk, srcPath, 6);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_01_extract.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    char dstPath[256];
    sprintf(dstPath, "test_dst_extract.txt");
    ASSERT_EQ(xpkExtractFile(xpk, 0, dstPath), 0);

    fp = fopen(dstPath, "rb");
    char readData[256];
    size_t readSize = fread(readData, 1, sizeof(readData), fp);
    fclose(fp);

    ASSERT_EQ(readSize, sizeof(testData));
    ASSERT_EQ(memcmp(readData, testData, readSize), 0);

    xpkClose(xpk);

    xrtFileDelete(srcPath);
    xrtFileDelete(dstPath);
}

TEST(core_extract_data) {
    xpkObject xpk = xpkOpen("test_01_extract_data.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    char data[] = "Extract me!";
    xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_01_extract_data.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    uint32_t outSize = 0;
    void* extracted = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(extracted);
    ASSERT_EQ(outSize, strlen(data));
    ASSERT_EQ(memcmp(extracted, data, outSize), 0);
    xpkFree(extracted);

    xpkClose(xpk);
}

TEST(core_multiple_operations) {
    xpkObject xpk = xpkOpen("test_01_multi_ops.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 3; i++) {
        char data[64];
        sprintf(data, "Operation %d", i);
        xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_01_multi_ops.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    xpkAppendData(xpk, "New file", 8, 6);
    ASSERT_EQ(xpkCount(xpk), 4);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_01_multi_ops.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 4);
    xpkClose(xpk);
}

int main(void) {
    xrtInit();

    printf("=================================================\n");
    printf("  xPack Ver7 - Core Basic Operations Test\n");
    printf("=================================================\n\n");

    printf("[Initialization Tests]\n");
    RUN_TEST(core_initialization);
    RUN_TEST(core_create_empty_package);
    printf("\n");

    printf("[Add File Tests]\n");
    RUN_TEST(core_add_file);
    RUN_TEST(core_add_directory);
    RUN_TEST(core_add_data);
    printf("\n");

    printf("[Save/Load Tests]\n");
    RUN_TEST(core_save_load);
    RUN_TEST(core_extract_file);
    RUN_TEST(core_extract_data);
    printf("\n");

    printf("[Compression Algorithm Tests]\n");
    RUN_TEST(core_compression_lz4);
    RUN_TEST(core_compression_zstd);
    RUN_TEST(core_compression_lzma2);
    RUN_TEST(core_compression_all_algorithms);
    printf("\n");

    printf("[Package Property Tests]\n");
    RUN_TEST(core_package_properties);
    RUN_TEST(core_file_count);
    RUN_TEST(core_traverse_files);
    RUN_TEST(core_multiple_operations);
    printf("\n");

    printf("=================================================\n");
    printf("  Results: %d passed, %d failed\n", tests_passed, tests_failed);
    printf("=================================================\n");

    return tests_failed > 0 ? 1 : 0;
}
