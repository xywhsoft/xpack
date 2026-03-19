/*
 * xPack Ver7 - Compression Ratio (29)
 */

#include "test_framework.h"

static char* createTestData_29(int size, char pattern) {
    char* pData = (char*)malloc(size);
    for (int i = 0; i < size; i++) {
        pData[i] = (char)(pattern + (i % 26));
    }
    return pData;
}

TEST(compression_ratio_repeated_data) {
    const char* sFilename = "test_29_ratio_repeated.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    char* pData = createTestData_29(1024 * 1024, 'A');
    ASSERT_NE(xpkAppendData(xpk, pData, 1024 * 1024, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);

    xpkStat stat;
    ASSERT_EQ(xpkStatGet(xpk, &stat), 0);

    ASSERT_EQ(stat.totalSize, 1024 * 1024);
    ASSERT_NE(stat.packedSize, 0);

    float fRatio = (stat.packedSize * 100.0f) / stat.totalSize;
    ASSERT_LT(fRatio, 50.0f);

    free(pData);
    xpkClose(xpk);
}

TEST(compression_ratio_random_data) {
    const char* sFilename = "test_29_ratio_random.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    char* pData = createTestData_29(1024 * 1024, 'R');
    for (int i = 0; i < 1024 * 1024; i++) {
        pData[i] = (char)(rand() % 256);
    }

    ASSERT_NE(xpkAppendData(xpk, pData, 1024 * 1024, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);

    xpkStat stat;
    ASSERT_EQ(xpkStatGet(xpk, &stat), 0);

    ASSERT_EQ(stat.totalSize, 1024 * 1024);
    ASSERT_NE(stat.packedSize, 0);

    free(pData);
    xpkClose(xpk);
}

TEST(compression_ratio_zero_data) {
    const char* sFilename = "test_29_ratio_zero.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    char* pData = createTestData_29(1024 * 1024, '\0');
    ASSERT_NE(xpkAppendData(xpk, pData, 1024 * 1024, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);

    xpkStat stat;
    ASSERT_EQ(xpkStatGet(xpk, &stat), 0);

    ASSERT_EQ(stat.totalSize, 1024 * 1024);
    ASSERT_NE(stat.packedSize, 0);

    float fRatio = (stat.packedSize * 100.0f) / stat.totalSize;
    ASSERT_LT(fRatio, 10.0f);

    free(pData);
    xpkClose(xpk);
}

TEST(compression_ratio_text_data) {
    const char* sFilename = "test_29_ratio_text.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    char* pData = createTestData_29(1024 * 1024, 'T');
    for (int i = 0; i < 1024 * 1024; i++) {
        pData[i] = (char)(32 + (rand() % 64));
    }

    ASSERT_NE(xpkAppendData(xpk, pData, 1024 * 1024, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);

    xpkStat stat;
    ASSERT_EQ(xpkStatGet(xpk, &stat), 0);

    ASSERT_EQ(stat.totalSize, 1024 * 1024);
    ASSERT_NE(stat.packedSize, 0);

    free(pData);
    xpkClose(xpk);
}

TEST(compression_ratio_json_data) {
    const char* sFilename = "test_29_ratio_json.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    char* pData = createTestData_29(1024 * 1024, 'J');
    for (int i = 0; i < 1024 * 1024; i++) {
        pData[i] = (char)(32 + (rand() % 95));
        if (i % 100 == 0) {
            pData[i] = '{';
        } else if (i % 100 == 50) {
            pData[i] = ':';
        } else if (i % 100 == 99) {
            pData[i] = '}';
        }
    }

    ASSERT_NE(xpkAppendData(xpk, pData, 1024 * 1024, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);

    xpkStat stat;
    ASSERT_EQ(xpkStatGet(xpk, &stat), 0);

    ASSERT_EQ(stat.totalSize, 1024 * 1024);
    ASSERT_NE(stat.packedSize, 0);

    free(pData);
    xpkClose(xpk);
}

TEST(compression_ratio_xml_data) {
    const char* sFilename = "test_29_ratio_xml.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    char* pData = createTestData_29(1024 * 1024, 'X');
    for (int i = 0; i < 1024 * 1024; i++) {
        pData[i] = (char)(32 + (rand() % 95));
        if (i % 100 == 0) {
            pData[i] = '<';
        } else if (i % 100 == 50) {
            pData[i] = '>';
        }
    }

    ASSERT_NE(xpkAppendData(xpk, pData, 1024 * 1024, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);

    xpkStat stat;
    ASSERT_EQ(xpkStatGet(xpk, &stat), 0);

    ASSERT_EQ(stat.totalSize, 1024 * 1024);
    ASSERT_NE(stat.packedSize, 0);

    free(pData);
    xpkClose(xpk);
}

TEST(compression_ratio_binary_data) {
    const char* sFilename = "test_29_ratio_binary.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    char* pData = createTestData_29(1024 * 1024, 'B');
    for (int i = 0; i < 1024 * 1024; i++) {
        pData[i] = (char)(rand() % 256);
    }

    ASSERT_NE(xpkAppendData(xpk, pData, 1024 * 1024, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);

    xpkStat stat;
    ASSERT_EQ(xpkStatGet(xpk, &stat), 0);

    ASSERT_EQ(stat.totalSize, 1024 * 1024);
    ASSERT_NE(stat.packedSize, 0);

    free(pData);
    xpkClose(xpk);
}

TEST(compression_ratio_mixed_files) {
    const char* sFilename = "test_29_ratio_mixed.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    char* pRepeated = createTestData_29(1024 * 1024, 'A');
    char* pRandom = createTestData_29(512 * 1024, 'R');
    for (int i = 0; i < 512 * 1024; i++) {
        pRandom[i] = (char)(rand() % 256);
    }
    char* pZero = createTestData_29(256 * 1024, '\0');

    ASSERT_NE(xpkAppendData(xpk, pRepeated, 1024 * 1024, 1), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk, pRandom, 512 * 1024, 2), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk, pZero, 256 * 1024, 3), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);

    xpkStat stat;
    ASSERT_EQ(xpkStatGet(xpk, &stat), 0);

    ASSERT_EQ(stat.totalSize, 1024 * 1024 + 512 * 1024 + 256 * 1024);
    ASSERT_NE(stat.packedSize, 0);

    free(pRepeated);
    free(pRandom);
    free(pZero);
    xpkClose(xpk);
}

TEST(compression_ratio_size_impact) {
    const char* sFilename = "test_29_ratio_size.xpk";

    int sizes[] = {1024, 10240, 102400, 1048576};

    for (int i = 0; i < 4; i++) {
        char sSizeFilename[64];
        sprintf(sSizeFilename, "%s_%d", sFilename, i);

        xpkObject xpk = xpkOpen(sSizeFilename, XPK_TYPE_CORE, 0);
        ASSERT_NOT_NULL(xpk);

        char* pData = createTestData_29(sizes[i], 'S');
        ASSERT_NE(xpkAppendData(xpk, pData, sizes[i], 1), UINT32_MAX);
        ASSERT_EQ(xpkSave(xpk), 0);

        xpkStat stat;
        ASSERT_EQ(xpkStatGet(xpk, &stat), 0);

        ASSERT_EQ(stat.totalSize, sizes[i]);
        ASSERT_NE(stat.packedSize, 0);

        free(pData);
        xpkClose(xpk);
    }
}

TEST(compression_ratio_after_update) {
    const char* sFilename = "test_29_ratio_update.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    char* pOriginal = createTestData_29(1024 * 1024, 'O');
    ASSERT_NE(xpkAppendData(xpk, pOriginal, 1024 * 1024, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);

    xpkStat stat1;
    ASSERT_EQ(xpkStatGet(xpk, &stat1), 0);

    char* pUpdated = createTestData_29(2048 * 1024, 'U');
    ASSERT_EQ(xpkUpdateData(xpk, 0, pUpdated, 2048 * 1024, 6), 0);
    ASSERT_EQ(xpkSave(xpk), 0);

    xpkStat stat2;
    ASSERT_EQ(xpkStatGet(xpk, &stat2), 0);

    ASSERT_EQ(stat2.totalSize, 2048 * 1024);
    ASSERT_NE(stat2.packedSize, 0);

    free(pOriginal);
    free(pUpdated);
    xpkClose(xpk);
}

TEST(compression_ratio_after_rebuild) {
    const char* sFilename = "test_29_ratio_rebuild.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 100; i++) {
        char* pData = createTestData_29(10240, 'R' + (i % 26));
        ASSERT_NE(xpkAppendData(xpk, pData, 10240, i + 1), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);

    xpkStat stat1;
    ASSERT_EQ(xpkStatGet(xpk, &stat1), 0);

    for (int i = 0; i < 50; i++) {
        ASSERT_EQ(xpkRemove(xpk, i * 2), 0);
    }

    ASSERT_EQ(xpkRebuild(xpk), 0);
    ASSERT_EQ(xpkSave(xpk), 0);

    xpkStat stat2;
    ASSERT_EQ(xpkStatGet(xpk, &stat2), 0);

    ASSERT_EQ(stat2.totalSize, 50 * 10240);
    ASSERT_NE(stat2.packedSize, 0);

    xpkClose(xpk);
}

void register_29_compression_ratio_tests(void) {
    TEST_REGISTER(compression_ratio_repeated_data, CAT_COMPRESSION, "Compression ratio repeated data");
    TEST_REGISTER(compression_ratio_random_data, CAT_COMPRESSION, "Compression ratio random data");
    TEST_REGISTER(compression_ratio_zero_data, CAT_COMPRESSION, "Compression ratio zero data");
    TEST_REGISTER(compression_ratio_text_data, CAT_COMPRESSION, "Compression ratio text data");
    TEST_REGISTER(compression_ratio_json_data, CAT_COMPRESSION, "Compression ratio json data");
    TEST_REGISTER(compression_ratio_xml_data, CAT_COMPRESSION, "Compression ratio xml data");
    TEST_REGISTER(compression_ratio_binary_data, CAT_COMPRESSION, "Compression ratio binary data");
    TEST_REGISTER(compression_ratio_mixed_files, CAT_COMPRESSION, "Compression ratio mixed files");
    TEST_REGISTER(compression_ratio_size_impact, CAT_COMPRESSION, "Compression ratio size impact");
    TEST_REGISTER(compression_ratio_after_update, CAT_COMPRESSION, "Compression ratio after update");
    TEST_REGISTER(compression_ratio_after_rebuild, CAT_COMPRESSION, "Compression ratio after rebuild");
}
