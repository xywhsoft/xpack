/*
 * xPack Ver7 - 固实压缩测试 (10)
 */

#include "test_framework.h"

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

    uint32_t solidSize = 0;
    for (uint32_t i = 0; i < xpkCount(xpkSolid); i++) {
        solidSize += xpkInfoPacked(xpkSolid, i);
    }
    xpkClose(xpkSolid);

    ASSERT_GE(normalSize, solidSize);
    ASSERT_GT(solidSize, 0);
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

TEST(solid_disable_mode) {
    xpkObject xpk = xpkOpen("test_10_solid_disable.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkSolidModeSet(xpk, 1), 0);
    ASSERT_EQ(xpkSolidMode(xpk), 1);

    ASSERT_EQ(xpkSolidModeSet(xpk, 0), 0);
    ASSERT_EQ(xpkSolidMode(xpk), 0);

    xpkClose(xpk);
}

void register_10_solid_compression_tests(void) {
    TEST_REGISTER(solid_create_basic, CAT_SOLID, "Test basic solid compression");
    TEST_REGISTER(solid_add_files, CAT_SOLID, "Test add files in solid mode");
    TEST_REGISTER(solid_traverse, CAT_SOLID, "Test traverse solid files");
    TEST_REGISTER(solid_compression_ratio_vs_normal, CAT_SOLID, "Test compression ratio vs normal");
    TEST_REGISTER(solid_empty_files, CAT_SOLID, "Test empty files in solid mode");
    TEST_REGISTER(solid_large_files, CAT_SOLID, "Test large files in solid mode");
    TEST_REGISTER(solid_compression_levels, CAT_SOLID, "Test solid compression levels");
    TEST_REGISTER(solid_mixed_sizes, CAT_SOLID, "Test mixed size files in solid mode");
    TEST_REGISTER(solid_disable_mode, CAT_SOLID, "Test disable solid mode");
}
