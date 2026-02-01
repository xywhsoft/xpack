/*
 * xPack Ver7 - Regression Tests (30)
 */

#include "test_framework.h"

static char* createTestData_30(int size, char pattern) {
    char* pData = (char*)malloc(size);
    for (int i = 0; i < size; i++) {
        pData[i] = (char)(pattern + (i % 26));
    }
    return pData;
}

TEST(regression_basic_operations) {
    char* pData = createTestData_30(1024, 'A');
    const char* sFilename = "test_30_basic.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_NE(xpkAppendData(xpk, pData, 1024, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);

    uint32_t outSize = 0;
    void* pExtracted = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(pExtracted);
    ASSERT_EQ(outSize, 1024);
    ASSERT_EQ(memcmp(pData, pExtracted, 1024), 0);
    xpkFree(pExtracted);

    free(pData);
    xpkClose(xpk);
}

TEST(regression_multiple_files) {
    const char* sFilename = "test_30_multi.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 50; i++) {
        char* pData = createTestData_30(512, 'A' + (i % 26));
        ASSERT_NE(xpkAppendData(xpk, pData, 512, i), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkCount(xpk), 50);
    ASSERT_EQ(xpkVerifyAll(xpk), 0);

    xpkClose(xpk);
}

TEST(regression_index_mode) {
    char* pData = createTestData_30(2048, 'B');
    const char* sFilename = "test_30_index.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_NE(xpkAppendData(xpk, pData, 2048, 100), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk, pData, 2048, 200), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_NE(xpkIndexFind(xpk, 100), UINT32_MAX);
    ASSERT_NE(xpkIndexFind(xpk, 200), UINT32_MAX);
    ASSERT_EQ(xpkIndexFind(xpk, 300), UINT32_MAX);

    uint32_t outSize = 0;
    void* pExtracted = xpkExtractData(xpk, 100, &outSize);
    ASSERT_NOT_NULL(pExtracted);
    ASSERT_EQ(outSize, 2048);
    ASSERT_EQ(memcmp(pData, pExtracted, 2048), 0);
    xpkFree(pExtracted);

    free(pData);
    xpkClose(xpk);
}

TEST(regression_path_mode) {
    char* pData = createTestData_30(1024, 'C');
    const char* sFilename = "test_30_path.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_LINUX, 0);
    ASSERT_NOT_NULL(xpk);
    uint32_t pos = xpkPathAppendData(xpk, "folder/file.txt", pData, 1024, 1);
    ASSERT_NE(pos, UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_NE(xpkPathFind(xpk, "folder/file.txt"), UINT32_MAX);

    uint32_t outSize = 0;
    void* pExtracted = xpkPathExtractData(xpk, "folder/file.txt", &outSize);
    ASSERT_NOT_NULL(pExtracted);
    ASSERT_EQ(outSize, 1024);
    ASSERT_EQ(memcmp(pData, pExtracted, 1024), 0);
    xpkFree(pExtracted);

    free(pData);
    xpkClose(xpk);
}

TEST(regression_remove_update) {
    char* pData = createTestData_30(2048, 'D');
    const char* sFilename = "test_30_remove_update.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_NE(xpkAppendData(xpk, pData, 2048, 1), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk, pData, 2048, 2), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk, pData, 2048, 3), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_EQ(xpkRemove(xpk, 1), 0);
    ASSERT_EQ(xpkRemove(xpk, 2), 0);

    ASSERT_EQ(xpkUpdateData(xpk, 0, pData, 2048, 0), 0);
    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_EQ(xpkCount(xpk), 1);
    ASSERT_EQ(xpkVerifyAll(xpk), 0);

    free(pData);
    xpkClose(xpk);
}

TEST(regression_compression_levels) {
    const char* sFilename = "test_30_compress.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    char* pData1 = createTestData_30(1024, 'E');
    char* pData2 = createTestData_30(1024, 'F');
    char* pData3 = createTestData_30(1024, 'G');

    ASSERT_NE(xpkAppendData(xpk, pData1, 1024, 1), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk, pData2, 1024, 2), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk, pData3, 1024, 3), UINT32_MAX);

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkVerifyAll(xpk), 0);

    free(pData1);
    free(pData2);
    free(pData3);
    xpkClose(xpk);
}

TEST(regression_large_file) {
    const char* sFilename = "test_30_large.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    char* pData = createTestData_30(10 * 1024 * 1024, 'H');
    ASSERT_NE(xpkAppendData(xpk, pData, 10 * 1024 * 1024, 1), UINT32_MAX);

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkVerifyAll(xpk), 0);

    free(pData);
    xpkClose(xpk);
}

TEST(regression_many_files) {
    const char* sFilename = "test_30_many.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 200; i++) {
        char* pData = createTestData_30(512, 'I' + (i % 26));
        ASSERT_NE(xpkAppendData(xpk, pData, 512, i), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkCount(xpk), 200);
    ASSERT_EQ(xpkVerifyAll(xpk), 0);

    xpkClose(xpk);
}

TEST(regression_traverse_operations) {
    const char* sFilename = "test_30_traverse.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 10; i++) {
        char* pData = createTestData_30(2048, 'J' + (i % 26));
        ASSERT_NE(xpkAppendData(xpk, pData, 2048, i * 10), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);

    int iCount = 0;
    ASSERT_EQ(xpkEach(xpk, NULL, &iCount), 0);
    ASSERT_EQ(iCount, 10);

    ASSERT_EQ(xpkCount(xpk), 10);
    xpkClose(xpk);
}

TEST(regression_statistics) {
    char* pData = createTestData_30(8192, 'K');
    const char* sFilename = "test_30_stats.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_NE(xpkAppendData(xpk, pData, 8192, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);

    xpkStat stat;
    ASSERT_EQ(xpkStatGet(xpk, &stat), 0);
    ASSERT_EQ(stat.totalSize, 8192);
    ASSERT_NE(stat.packedSize, 0);

    free(pData);
    xpkClose(xpk);
}

TEST(regression_rebuild) {
    const char* sFilename = "test_30_rebuild.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 15; i++) {
        char* pData = createTestData_30(2048, 'L' + (i % 26));
        ASSERT_NE(xpkAppendData(xpk, pData, 2048, i), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_EQ(xpkRemove(xpk, 0), 0);
    ASSERT_EQ(xpkRemove(xpk, 0), 0);
    ASSERT_EQ(xpkRemove(xpk, 0), 0);

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkRebuild(xpk), 0);

    ASSERT_EQ(xpkCount(xpk), 12);
    ASSERT_EQ(xpkVerifyAll(xpk), 0);

    xpkClose(xpk);
}

TEST(regression_properties) {
    const char* sFilename = "test_30_props.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkType(xpk), XPK_TYPE_CORE);
    ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);
    ASSERT_EQ(xpkType(xpk), XPK_TYPE_INDEX);

    ASSERT_EQ(xpkDiscCodeSet(xpk, 0x12345678), 0);
    ASSERT_EQ(xpkDiscCode(xpk), 0x12345678);

    xpkClose(xpk);
}

TEST(regression_extract_all) {
    const char* sFilename = "test_30_extract_all.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 20; i++) {
        char* pData = createTestData_30(4096, 'M' + (i % 26));
        ASSERT_NE(xpkAppendData(xpk, pData, 4096, i + 1), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkExtractAll(xpk, "."), 0);
    ASSERT_EQ(xpkVerifyAll(xpk), 0);

    ASSERT_EQ(xpkCount(xpk), 20);

    xpkClose(xpk);
}

TEST(regression_user_data) {
    char* pData = createTestData_30(1024, 'N');
    char* pUserData = createTestData_30(256, 'O');
    const char* sFilename = "test_30_userdata.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_NE(xpkAppendData(xpk, pData, 1024, 100), UINT32_MAX);
    ASSERT_EQ(xpkIndexUserDataSet(xpk, 100, 256), 0);
    ASSERT_EQ(xpkSave(xpk), 0);

    int32_t userData = xpkIndexUserData(xpk, 100);
    ASSERT_EQ(userData, 256);

    free(pData);
    free(pUserData);
    xpkClose(xpk);
}

TEST(regression_edge_cases) {
    const char* sFilename = "test_30_edge.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    char* pData1 = createTestData_30(1, 'A');
    char* pData2 = createTestData_30(1024, 'B');
    char* pData3 = createTestData_30(4 * 1024 * 1024, 'C');

    ASSERT_NE(xpkAppendData(xpk, pData1, 1, 1), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk, pData2, 1024, 2), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk, pData3, 4 * 1024 * 1024, 3), UINT32_MAX);

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkVerifyAll(xpk), 0);
    ASSERT_EQ(xpkCount(xpk), 3);

    free(pData1);
    free(pData2);
    free(pData3);
    xpkClose(xpk);
}

TEST(regression_save_load_cycles) {
    const char* sFilename = "test_30_cycles.xpk";
    char* pData = createTestData_30(2048, 'P');

    for (int iCycle = 0; iCycle < 10; iCycle++) {
        xpkObject xpk = xpkOpen(sFilename, 0, 0);
        ASSERT_NOT_NULL(xpk);

        if (iCycle == 0) {
            for (int j = 0; j < 20; j++) {
                ASSERT_NE(xpkAppendData(xpk, pData, 2048, j), UINT32_MAX);
            }
        }

        ASSERT_EQ(xpkCount(xpk), 20);
        ASSERT_EQ(xpkSave(xpk), 0);
        ASSERT_EQ(xpkVerifyAll(xpk), 0);
        xpkClose(xpk);
    }

    free(pData);
}

void register_30_regression_tests(void) {
    TEST_REGISTER(regression_basic_operations, CAT_CORE, "Regression basic operations");
    TEST_REGISTER(regression_multiple_files, CAT_CORE, "Regression multiple files");
    TEST_REGISTER(regression_index_mode, CAT_CORE, "Regression index mode");
    TEST_REGISTER(regression_path_mode, CAT_CORE, "Regression path mode");
    TEST_REGISTER(regression_remove_update, CAT_CORE, "Regression remove update");
    TEST_REGISTER(regression_compression_levels, CAT_CORE, "Regression compression levels");
    TEST_REGISTER(regression_large_file, CAT_CORE, "Regression large file");
    TEST_REGISTER(regression_many_files, CAT_CORE, "Regression many files");
    TEST_REGISTER(regression_traverse_operations, CAT_CORE, "Regression traverse operations");
    TEST_REGISTER(regression_statistics, CAT_CORE, "Regression statistics");
    TEST_REGISTER(regression_rebuild, CAT_CORE, "Regression rebuild");
    TEST_REGISTER(regression_properties, CAT_CORE, "Regression properties");
    TEST_REGISTER(regression_extract_all, CAT_CORE, "Regression extract all");
    TEST_REGISTER(regression_user_data, CAT_CORE, "Regression user data");
    TEST_REGISTER(regression_edge_cases, CAT_CORE, "Regression edge cases");
    TEST_REGISTER(regression_save_load_cycles, CAT_CORE, "Regression save load cycles");
}
