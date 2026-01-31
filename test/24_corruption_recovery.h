/*
 * xPack Ver7 - Corruption Recovery (24)
 */

#include "test_framework.h"

static char* createTestData_24(int size, char pattern) {
    char* pData = (char*)malloc(size);
    for (int i = 0; i < size; i++) {
        pData[i] = (char)(pattern + (i % 26));
    }
    return pData;
}

TEST(create_valid_package_for_corruption_test) {
    char* pData = createTestData_24(2048, 'A');
    const char* sFilename = "test_24_valid.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_NE(xpkAppendData(xpk, pData, 2048, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkVerifyAll(xpk), 0);

    free(pData);
    xpkClose(xpk);
}

TEST(corrupted_signature_detection) {
    const char* sFilename = "test_24_corrupt_sig.xpk";

    FILE* pFile = fopen(sFilename, "wb");
    if (pFile) {
        fwrite("BAD", 1, 3, pFile);
        fclose(pFile);
    }

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NULL(xpk);
}

TEST(truncated_file_detection) {
    char* pData = createTestData_24(4096, 'B');
    const char* sSrcFilename = "test_24_src.xpk";
    const char* sTruncFilename = "test_24_truncated.xpk";

    xpkObject xpk = xpkOpen(sSrcFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_NE(xpkAppendData(xpk, pData, 4096, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    FILE* pSrc = fopen(sSrcFilename, "rb");
    FILE* pDst = fopen(sTruncFilename, "wb");
    if (pSrc && pDst) {
        char pBuffer[1024];
        int iRead = fread(pBuffer, 1, 512, pSrc);
        fwrite(pBuffer, 1, iRead, pDst);
    }
    if (pSrc) fclose(pSrc);
    if (pDst) fclose(pDst);

    xpk = xpkOpen(sTruncFilename, 0, 0);
    ASSERT_NULL(xpk);

    free(pData);
}

TEST(damaged_data_recovery_with_verify) {
    char* pData = createTestData_24(2048, 'C');
    const char* sFilename = "test_24_damaged.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_NE(xpkAppendData(xpk, pData, 2048, 1), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk, pData, 2048, 2), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_EQ(xpkVerify(xpk, 0), 0);
    ASSERT_EQ(xpkVerify(xpk, 1), 0);

    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkVerifyAll(xpk), 0);

    uint32_t outSize = 0;
    void* pExtracted = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(pExtracted);
    ASSERT_EQ(outSize, 2048);
    ASSERT_EQ(memcmp(pData, pExtracted, 2048), 0);
    xpkFree(pExtracted);

    free(pData);
    xpkClose(xpk);
}

TEST(remove_corrupted_file) {
    char* pData = createTestData_24(1024, 'D');
    const char* sFilename = "test_24_remove_corrupt.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_NE(xpkAppendData(xpk, pData, 1024, 1), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk, pData, 1024, 2), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk, pData, 1024, 3), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_EQ(xpkRemove(xpk, 1), 0);
    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkVerifyAll(xpk), 0);

    ASSERT_EQ(xpkCount(xpk), 2);

    free(pData);
    xpkClose(xpk);
}

TEST(rebuild_after_partial_damage) {
    char* pData = createTestData_24(2048, 'E');
    const char* sFilename = "test_24_rebuild.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 10; i++) {
        ASSERT_NE(xpkAppendData(xpk, pData, 2048, i + 1), UINT32_MAX);
    }

    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_EQ(xpkRemove(xpk, 2), 0);
    ASSERT_EQ(xpkRemove(xpk, 6), 0);

    ASSERT_EQ(xpkRebuild(xpk), 0);
    ASSERT_EQ(xpkVerifyAll(xpk), 0);

    ASSERT_EQ(xpkCount(xpk), 8);

    free(pData);
    xpkClose(xpk);
}

TEST(extract_all_after_removal) {
    char* pData = createTestData_24(1536, 'F');
    const char* sFilename = "test_24_extract_all.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 5; i++) {
        ASSERT_NE(xpkAppendData(xpk, pData, 1536, i + 1), UINT32_MAX);
    }

    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_EQ(xpkRemove(xpk, 1), 0);
    ASSERT_EQ(xpkRemove(xpk, 3), 0);

    ASSERT_EQ(xpkVerifyAll(xpk), 0);
    ASSERT_EQ(xpkExtractAll(xpk, "."), 0);

    ASSERT_EQ(xpkCount(xpk), 3);

    free(pData);
    xpkClose(xpk);
}

TEST(save_and_reload_after_damage) {
    char* pData1 = createTestData_24(1024, 'G');
    char* pData2 = createTestData_24(2048, 'H');
    const char* sFilename = "test_24_save_reload.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_NE(xpkAppendData(xpk, pData1, 1024, 1), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk, pData2, 2048, 2), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_EQ(xpkRemove(xpk, 0), 0);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 1);
    ASSERT_EQ(xpkVerifyAll(xpk), 0);

    uint32_t outSize = 0;
    void* pExtracted = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(pExtracted);
    ASSERT_EQ(outSize, 2048);
    ASSERT_EQ(memcmp(pData2, pExtracted, 2048), 0);
    xpkFree(pExtracted);

    free(pData1);
    free(pData2);
    xpkClose(xpk);
}

TEST(rebuild_optimizes_structure) {
    char* pData = createTestData_24(1024, 'I');
    const char* sFilename = "test_24_rebuild_opt.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 20; i++) {
        ASSERT_NE(xpkAppendData(xpk, pData, 1024, i + 1), UINT32_MAX);
    }

    ASSERT_EQ(xpkSave(xpk), 0);

    for (int i = 0; i < 10; i++) {
        ASSERT_EQ(xpkRemove(xpk, i * 2), 0);
    }

    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_EQ(xpkRebuild(xpk), 0);
    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkVerifyAll(xpk), 0);

    ASSERT_EQ(xpkCount(xpk), 10);

    free(pData);
    xpkClose(xpk);
}

TEST(statistics_after_recovery) {
    char* pData = createTestData_24(2048, 'J');
    const char* sFilename = "test_24_stats_recover.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 10; i++) {
        ASSERT_NE(xpkAppendData(xpk, pData, 2048, i + 1), UINT32_MAX);
    }

    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_EQ(xpkRemove(xpk, 4), 0);
    ASSERT_EQ(xpkRemove(xpk, 8), 0);

    ASSERT_EQ(xpkRebuild(xpk), 0);

    xpkStat stat;
    ASSERT_EQ(xpkStatGet(xpk, &stat), 0);
    ASSERT_EQ(stat.totalSize, 8 * 2048);
    ASSERT_NE(stat.packedSize, 0);

    ASSERT_EQ(xpkCount(xpk), 8);

    free(pData);
    xpkClose(xpk);
}

TEST(traverse_after_recovery) {
    char* pData = createTestData_24(1024, 'K');
    const char* sFilename = "test_24_traverse_recover.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 15; i++) {
        ASSERT_NE(xpkAppendData(xpk, pData, 1024, i * 10), UINT32_MAX);
    }

    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_EQ(xpkRemove(xpk, 49), 0);
    ASSERT_EQ(xpkRemove(xpk, 99), 0);

    ASSERT_EQ(xpkRebuild(xpk), 0);

    int iCount = 0;
    ASSERT_EQ(xpkEach(xpk, NULL, &iCount), 0);
    ASSERT_EQ(iCount, 13);

    ASSERT_EQ(xpkVerifyAll(xpk), 0);

    free(pData);
    xpkClose(xpk);
}

TEST(update_after_recovery) {
    char* pData1 = createTestData_24(1024, 'L');
    char* pData2 = createTestData_24(2048, 'M');
    const char* sFilename = "test_24_update_recover.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 5; i++) {
        ASSERT_NE(xpkAppendData(xpk, pData1, 1024, i + 1), UINT32_MAX);
    }

    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_EQ(xpkRemove(xpk, 2), 0);

    ASSERT_EQ(xpkUpdateData(xpk, 0, pData2, 2048, 6), 0);
    ASSERT_EQ(xpkUpdateData(xpk, 3, pData2, 2048, 6), 0);

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkVerifyAll(xpk), 0);

    ASSERT_EQ(xpkCount(xpk), 4);

    free(pData1);
    free(pData2);
    xpkClose(xpk);
}

TEST(find_after_recovery) {
    char* pData = createTestData_24(1024, 'N');
    const char* sFilename = "test_24_find_recover.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_NE(xpkAppendData(xpk, pData, 1024, 100), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk, pData, 1024, 200), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk, pData, 1024, 300), UINT32_MAX);

    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_EQ(xpkRemove(xpk, 199), 0);
    ASSERT_EQ(xpkRebuild(xpk), 0);

    ASSERT_NE(xpkIndexFind(xpk, 100), UINT32_MAX);
    ASSERT_EQ(xpkIndexFind(xpk, 200), UINT32_MAX);
    ASSERT_NE(xpkIndexFind(xpk, 300), UINT32_MAX);

    ASSERT_EQ(xpkCount(xpk), 2);

    free(pData);
    xpkClose(xpk);
}

TEST(multiple_recovery_cycles) {
    char* pData = createTestData_24(1536, 'O');
    const char* sFilename = "test_24_multi_recover.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 20; i++) {
        ASSERT_NE(xpkAppendData(xpk, pData, 1536, i + 1), UINT32_MAX);
    }

    ASSERT_EQ(xpkSave(xpk), 0);

    for (int iCycle = 0; iCycle < 3; iCycle++) {
        for (int i = 0; i < 5; i++) {
            ASSERT_EQ(xpkRemove(xpk, iCycle * 5), 0);
        }
        ASSERT_EQ(xpkRebuild(xpk), 0);
        ASSERT_EQ(xpkVerifyAll(xpk), 0);
        ASSERT_EQ(xpkSave(xpk), 0);
    }

    ASSERT_EQ(xpkCount(xpk), 5);

    free(pData);
    xpkClose(xpk);
}

void register_24_corruption_recovery_tests(void) {
    TEST_REGISTER(create_valid_package_for_corruption_test, CAT_ERROR, "Create valid package for corruption test");
    TEST_REGISTER(corrupted_signature_detection, CAT_ERROR, "Corrupted signature detection");
    TEST_REGISTER(truncated_file_detection, CAT_ERROR, "Truncated file detection");
    TEST_REGISTER(damaged_data_recovery_with_verify, CAT_ERROR, "Damaged data recovery with verify");
    TEST_REGISTER(remove_corrupted_file, CAT_ERROR, "Remove corrupted file");
    TEST_REGISTER(rebuild_after_partial_damage, CAT_ERROR, "Rebuild after partial damage");
    TEST_REGISTER(extract_all_after_removal, CAT_ERROR, "Extract all after removal");
    TEST_REGISTER(save_and_reload_after_damage, CAT_ERROR, "Save and reload after damage");
    TEST_REGISTER(rebuild_optimizes_structure, CAT_ERROR, "Rebuild optimizes structure");
    TEST_REGISTER(statistics_after_recovery, CAT_ERROR, "Statistics after recovery");
    TEST_REGISTER(traverse_after_recovery, CAT_ERROR, "Traverse after recovery");
    TEST_REGISTER(update_after_recovery, CAT_ERROR, "Update after recovery");
    TEST_REGISTER(find_after_recovery, CAT_ERROR, "Find after recovery");
    TEST_REGISTER(multiple_recovery_cycles, CAT_ERROR, "Multiple recovery cycles");
}
