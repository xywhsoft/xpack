/*
 * xPack Ver7 - 固实压缩测试
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
#define ASSERT_GE(a, b) ASSERT((a) >= (b))

TEST(solid_create_basic) {
    xpkObject xpk = xpkOpen("test_10_solid_basic.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkSolidModeSet(xpk, 1), 0);
    ASSERT_EQ(xpkSolidMode(xpk), 1);

    const char* data = "Solid compression test data";
    uint32_t pos = xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
    ASSERT_EQ(pos, 0);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_10_solid_basic.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkSolidMode(xpk), 1);

    uint32_t outSize = 0;
    void* extracted = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(extracted);
    ASSERT_EQ(outSize, strlen(data));
    ASSERT_EQ(memcmp(extracted, data, outSize), 0);
    xpkFree(extracted);

    xpkClose(xpk);
}

TEST(solid_add_files) {
    xpkObject xpk = xpkOpen("test_10_solid_files.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkSolidModeSet(xpk, 1), 0);

    char data[128];
    for (int i = 0; i < 5; i++) {
        sprintf(data, "File %d content with some repeated data", i);
        uint32_t pos = xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
        ASSERT_EQ(pos, i);
    }

    ASSERT_EQ(xpkCount(xpk), 5);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_10_solid_files.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 5);

    for (int i = 0; i < 5; i++) {
        char expected[128];
        sprintf(expected, "File %d content with some repeated data", i);

        uint32_t outSize = 0;
        void* extracted = xpkExtractData(xpk, i, &outSize);
        ASSERT_NOT_NULL(extracted);
        ASSERT_EQ(outSize, strlen(expected));
        ASSERT_EQ(memcmp(extracted, expected, outSize), 0);
        xpkFree(extracted);
    }

    xpkClose(xpk);
}

TEST(solid_extract_files) {
    xpkObject xpk = xpkOpen("test_10_solid_extract.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkSolidModeSet(xpk, 1), 0);

    for (int i = 0; i < 3; i++) {
        char data[64];
        sprintf(data, "Solid extract test %d", i);
        xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_10_solid_extract.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    for (uint32_t i = 0; i < xpkCount(xpk); i++) {
        uint32_t outSize = 0;
        void* extracted = xpkExtractData(xpk, i, &outSize);
        ASSERT_NOT_NULL(extracted);
        ASSERT_GT(outSize, 0);
        xpkFree(extracted);
    }

    xpkClose(xpk);
}

TEST(solid_traverse) {
    xpkObject xpk = xpkOpen("test_10_solid_traverse.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkSolidModeSet(xpk, 1), 0);

    for (int i = 0; i < 10; i++) {
        char data[32];
        sprintf(data, "Data %d", i);
        xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_10_solid_traverse.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    int count = 0;
    for (uint32_t i = 0; i < xpkCount(xpk); i++) {
        ASSERT_GT(xpkInfoSize(xpk, i), 0);
        ASSERT_GT(xpkInfoPacked(xpk, i), 0);
        count++;
    }

    ASSERT_EQ(count, 10);
    xpkClose(xpk);
}

TEST(solid_compression_ratio_vs_normal) {
    char dataFiles[5][2048];

    for (int i = 0; i < 5; i++) {
        memset(dataFiles[i], 'A' + (i % 3), sizeof(dataFiles[i]));
        strcpy(dataFiles[i] + 2000, "Common suffix string");
    }

    xpkObject xpkNormal = xpkOpen("test_10_ratio_normal.xpk", 0, 0);
    ASSERT_NOT_NULL(xpkNormal);

    for (int i = 0; i < 5; i++) {
        xpkAppendData(xpkNormal, dataFiles[i], sizeof(dataFiles[i]), 6);
    }
    ASSERT_EQ(xpkSave(xpkNormal), 0);

    uint32_t normalSize = 0;
    for (uint32_t i = 0; i < xpkCount(xpkNormal); i++) {
        normalSize += xpkInfoPacked(xpkNormal, i);
    }
    xpkClose(xpkNormal);

    xpkObject xpkSolid = xpkOpen("test_10_ratio_solid.xpk", 0, 0);
    ASSERT_NOT_NULL(xpkSolid);
    ASSERT_EQ(xpkSolidModeSet(xpkSolid, 1), 0);

    for (int i = 0; i < 5; i++) {
        xpkAppendData(xpkSolid, dataFiles[i], sizeof(dataFiles[i]), 6);
    }
    ASSERT_EQ(xpkSave(xpkSolid), 0);

    uint32_t solidOffset = 0;
    uint32_t solidSize = 0;
    ASSERT_EQ(xpkSolidBlockInfo(xpkSolid, &solidOffset, &solidSize), 0);
    xpkClose(xpkSolid);

    ASSERT_GT(normalSize, solidSize);
    ASSERT_GT(solidSize, 0);

    float improvement = 100.0f * (normalSize - solidSize) / normalSize;
    ASSERT_GT(improvement, 5.0f);
}

TEST(solid_empty_files) {
    xpkObject xpk = xpkOpen("test_10_solid_empty.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkSolidModeSet(xpk, 1), 0);

    uint32_t pos1 = xpkAppendData(xpk, NULL, 0, 6);
    ASSERT_EQ(pos1, 0);

    uint32_t pos2 = xpkAppendData(xpk, "", 0, 6);
    ASSERT_EQ(pos2, 1);

    const char* data = "Non-empty file";
    uint32_t pos3 = xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
    ASSERT_EQ(pos3, 2);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_10_solid_empty.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    uint32_t outSize = 0;
    void* extracted = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(extracted);
    ASSERT_EQ(outSize, 0);
    xpkFree(extracted);

    extracted = xpkExtractData(xpk, 1, &outSize);
    ASSERT_NOT_NULL(extracted);
    ASSERT_EQ(outSize, 0);
    xpkFree(extracted);

    extracted = xpkExtractData(xpk, 2, &outSize);
    ASSERT_NOT_NULL(extracted);
    ASSERT_EQ(outSize, strlen(data));
    ASSERT_EQ(memcmp(extracted, data, outSize), 0);
    xpkFree(extracted);

    xpkClose(xpk);
}

TEST(solid_large_files) {
    xpkObject xpk = xpkOpen("test_10_solid_large.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkSolidModeSet(xpk, 1), 0);

    size_t sizes[] = { 1024 * 1024, 2 * 1024 * 1024, 3 * 1024 * 1024 };
    uint32_t hashes[3];

    for (int i = 0; i < 3; i++) {
        void* data = malloc(sizes[i]);
        ASSERT_NOT_NULL(data);
        memset(data, 'A' + i, sizes[i]);

        hashes[i] = xrtHash32((ptr)data, sizes[i]);

        xpkAppendData(xpk, data, (uint32_t)sizes[i], 6);
        free(data);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_10_solid_large.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 3; i++) {
        uint32_t outSize = 0;
        void* extracted = xpkExtractData(xpk, i, &outSize);
        ASSERT_NOT_NULL(extracted);
        ASSERT_EQ(outSize, (uint32_t)sizes[i]);

        uint32_t hash = xrtHash32((ptr)extracted, outSize);
        ASSERT_EQ(hash, hashes[i]);

        xpkFree(extracted);
    }

    xpkClose(xpk);
}

TEST(solid_compression_levels) {
    for (int level = 1; level <= 12; level++) {
        char filename[64];
        sprintf(filename, "test_10_solid_level_%d.xpk", level);

        xpkObject xpk = xpkOpen(filename, 0, 0);
        ASSERT_NOT_NULL(xpk);

        ASSERT_EQ(xpkSolidModeSet(xpk, 1), 0);

        char data[1024];
        for (int i = 0; i < 3; i++) {
            sprintf(data, "Test file %d with repeated content", i);
            xpkAppendData(xpk, data, (uint32_t)strlen(data), level);
        }

        ASSERT_EQ(xpkSave(xpk), 0);
        xpkClose(xpk);

        xpk = xpkOpen(filename, 0, 1);
        ASSERT_NOT_NULL(xpk);

        for (int i = 0; i < 3; i++) {
            uint32_t outSize = 0;
            void* extracted = xpkExtractData(xpk, i, &outSize);
            ASSERT_NOT_NULL(extracted);
            xpkFree(extracted);
        }

        xpkClose(xpk);
    }
}

TEST(solid_mixed_sizes) {
    xpkObject xpk = xpkOpen("test_10_solid_mixed.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkSolidModeSet(xpk, 1), 0);

    size_t sizes[] = {0, 16, 256, 4096, 65536, 1048576};

    for (int i = 0; i < 6; i++) {
        void* data = malloc(sizes[i] ? sizes[i] : 1);
        ASSERT_NOT_NULL(data);
        if (sizes[i] > 0) {
            memset(data, 'A' + i, sizes[i]);
        }

        xpkAppendData(xpk, sizes[i] > 0 ? data : NULL, (uint32_t)sizes[i], 6);
        free(data);
    }

    ASSERT_EQ(xpkCount(xpk), 6);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_10_solid_mixed.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 6);

    for (int i = 0; i < 6; i++) {
        uint32_t outSize = 0;
        void* extracted = xpkExtractData(xpk, i, &outSize);
        ASSERT_NOT_NULL(extracted);
        ASSERT_EQ(outSize, (uint32_t)sizes[i]);

        if (sizes[i] > 0) {
            char* expected = (char*)malloc(sizes[i]);
            memset(expected, 'A' + i, sizes[i]);
            ASSERT_EQ(memcmp(extracted, expected, sizes[i]), 0);
            free(expected);
        }

        xpkFree(extracted);
    }

    xpkClose(xpk);
}

TEST(solid_path_mode) {
    xpkObject xpk = xpkOpen("test_10_solid_path.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkSolidModeSet(xpk, 1), 0);
    ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

    xpkPathAppendData(xpk, "dir1/file1.txt", "Content 1", 9, 6);
    xpkPathAppendData(xpk, "dir1/file2.txt", "Content 2", 9, 6);
    xpkPathAppendData(xpk, "dir2/file1.txt", "Content 3", 9, 6);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_10_solid_path.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkPathExists(xpk, "dir1/file1.txt"), 1);
    ASSERT_EQ(xpkPathExists(xpk, "dir1/file2.txt"), 1);
    ASSERT_EQ(xpkPathExists(xpk, "dir2/file1.txt"), 1);

    uint32_t outSize = 0;
    void* data = xpkPathExtractData(xpk, "dir1/file1.txt", &outSize);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(outSize, 9);
    ASSERT_EQ(memcmp(data, "Content 1", 9), 0);
    xpkFree(data);

    xpkClose(xpk);
}

TEST(solid_index_mode) {
    xpkObject xpk = xpkOpen("test_10_solid_index.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkSolidModeSet(xpk, 1), 0);
    ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

    xpkIndexAppendData(xpk, 100, "Index 100 data", 13, 6);
    xpkIndexAppendData(xpk, 200, "Index 200 data", 13, 6);
    xpkIndexAppendData(xpk, 150, "Index 150 data", 13, 6);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_10_solid_index.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    uint32_t pos = xpkIndexFind(xpk, 200);
    ASSERT_NE(pos, UINT32_MAX);

    uint32_t outSize = 0;
    void* data = xpkIndexExtractData(xpk, 200, &outSize);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(outSize, 13);
    ASSERT_EQ(memcmp(data, "Index 200 data", 13), 0);
    xpkFree(data);

    xpkClose(xpk);
}

TEST(solid_block_info) {
    xpkObject xpk = xpkOpen("test_10_solid_block.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkSolidModeSet(xpk, 1), 0);

    for (int i = 0; i < 5; i++) {
        char data[512];
        sprintf(data, "Block test file %d", i);
        xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
    }

    ASSERT_EQ(xpkSave(xpk), 0);

    uint32_t offset = 0;
    uint32_t size = 0;
    ASSERT_EQ(xpkSolidBlockInfo(xpk, &offset, &size), 0);

    ASSERT_NE(offset, 0);
    ASSERT_NE(size, 0);

    xpkClose(xpk);
}

TEST(solid_cannot_set_after_files) {
    xpkObject xpk = xpkOpen("test_10_solid_error.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    const char* data = "Test data";
    xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);

    ASSERT_NE(xpkSolidModeSet(xpk, 1), 0);
    ASSERT_EQ(xpkSolidMode(xpk), 0);

    xpkClose(xpk);
}

TEST(solid_disable_mode) {
    xpkObject xpk = xpkOpen("test_10_solid_disable.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkSolidModeSet(xpk, 1), 0);
    ASSERT_EQ(xpkSolidMode(xpk), 1);

    ASSERT_EQ(xpkSolidModeSet(xpk, 0), 0);
    ASSERT_EQ(xpkSolidMode(xpk), 0);

    xpkClose(xpk);
}

int main(void) {
    xrtInit();

    printf("=================================================\n");
    printf("  xPack Ver7 - Solid Compression Test\n");
    printf("=================================================\n\n");

    printf("[Basic Solid Compression Tests]\n");
    RUN_TEST(solid_create_basic);
    RUN_TEST(solid_add_files);
    RUN_TEST(solid_extract_files);
    RUN_TEST(solid_traverse);
    printf("\n");

    printf("[Solid Compression Ratio Tests]\n");
    RUN_TEST(solid_compression_ratio_vs_normal);
    printf("\n");

    printf("[Solid Edge Cases Tests]\n");
    RUN_TEST(solid_empty_files);
    RUN_TEST(solid_mixed_sizes);
    RUN_TEST(solid_large_files);
    printf("\n");

    printf("[Solid Compression Level Tests]\n");
    RUN_TEST(solid_compression_levels);
    printf("\n");

    printf("[Solid Mode Tests]\n");
    RUN_TEST(solid_path_mode);
    RUN_TEST(solid_index_mode);
    RUN_TEST(solid_block_info);
    RUN_TEST(solid_cannot_set_after_files);
    RUN_TEST(solid_disable_mode);
    printf("\n");

    printf("=================================================\n");
    printf("  Results: %d passed, %d failed\n", tests_passed, tests_failed);
    printf("=================================================\n");

    return tests_failed > 0 ? 1 : 0;
}
