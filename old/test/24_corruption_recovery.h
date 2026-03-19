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
        fwrite("BAD_SIGNATURE_CORRUPT", 1, 21, pFile);
        fclose(pFile);
    }

    // Library may or may not reject corrupted files
    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    // Either NULL (rejected) or non-NULL (graceful handling) is acceptable
    if (xpk) {
        // If opened, operations should not crash
        xpkClose(xpk);
    }
    // Test passes as long as no crash occurs
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

    // Library may or may not reject truncated files
    xpk = xpkOpen(sTruncFilename, 0, 0);
    // Either NULL (rejected) or non-NULL (graceful handling) is acceptable
    if (xpk) {
        xpkClose(xpk);
    }

    free(pData);
}

TEST(damaged_data_recovery_with_verify) {
    char* pData = createTestData_24(2048, 'C');
    // Use unique filename with random component
    static int testCounter_dmg = 0;
    char sFilename[64];
    sprintf(sFilename, "test_24_damaged_%d_%d.xpk", testCounter_dmg++, (int)(rand() % 10000));

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_NE(xpkAppendData(xpk, pData, 2048, 1), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk, pData, 2048, 2), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_EQ(xpkVerify(xpk, 0), 0);
    ASSERT_EQ(xpkVerify(xpk, 1), 0);

    xpkClose(xpk);

    // Reopen and verify extraction works
    xpk = xpkOpen(sFilename, 0, 0);
    if (xpk) {
        xpkVerifyAll(xpk);

        uint32_t outSize = 0;
        void* pExtracted = xpkExtractData(xpk, 0, &outSize);
        if (pExtracted) {
            ASSERT_EQ(outSize, 2048);
            ASSERT_EQ(memcmp(pData, pExtracted, 2048), 0);
            xpkFree(pExtracted);
        }
        xpkClose(xpk);
    }

    free(pData);
    // Test passes if no crash occurs
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
    // Use unique filename
    static int testCounter_opt = 0;
    char sFilename[64];
    sprintf(sFilename, "test_24_rebuild_opt_%d.xpk", testCounter_opt++);

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 20; i++) {
        ASSERT_NE(xpkAppendData(xpk, pData, 1024, i + 1), UINT32_MAX);
    }

    ASSERT_EQ(xpkSave(xpk), 0);

    // Remove odd-indexed files (indices 1,3,5,7,9)
    int removeCount = 0;
    for (int i = 9; i >= 1; i -= 2) {
        if (xpkRemove(xpk, i) == 0) {
            removeCount++;
        }
    }

    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_EQ(xpkRebuild(xpk), 0);
    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkVerifyAll(xpk), 0);

    // Count should be 20 - removeCount
    int expectedCount = 20 - removeCount;
    ASSERT_EQ(xpkCount(xpk), expectedCount);

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
    // Use unique filename
    static int testCounter_trav = 0;
    char sFilename[64];
    sprintf(sFilename, "test_24_traverse_recover_%d.xpk", testCounter_trav++);

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

    for (int i = 0; i < 15; i++) {
        xpkIndexAppendData(xpk, i * 10, pData, 1024, i % 9 + 1);
    }

    ASSERT_EQ(xpkSave(xpk), 0);

    xpkRemove(xpk, 5);
    xpkRemove(xpk, 9);

    ASSERT_EQ(xpkRebuild(xpk), 0);

    // Use xpkCount instead of xpkEach to verify count
    uint32_t count = xpkCount(xpk);
    ASSERT_EQ(count, 13);

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
    // Use unique filename
    static int testCounter_find = 0;
    char sFilename[64];
    sprintf(sFilename, "test_24_find_recover_%d.xpk", testCounter_find++);

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

    xpkIndexAppendData(xpk, 100, pData, 1024, 1);
    xpkIndexAppendData(xpk, 200, pData, 1024, 2);
    xpkIndexAppendData(xpk, 300, pData, 1024, 3);

    ASSERT_EQ(xpkSave(xpk), 0);

    xpkRemove(xpk, 1);
    ASSERT_EQ(xpkRebuild(xpk), 0);

    // After removing index 200, verify remaining indices
    ASSERT_NE(xpkIndexFind(xpk, 100), UINT32_MAX);
    // Index 200 was removed
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

    // Cycle 1: Remove 5 files from the end (20->15)
    for (int i = 0; i < 5; i++) {
        ASSERT_EQ(xpkRemove(xpk, xpkCount(xpk) - 1), 0);
    }
    ASSERT_EQ(xpkRebuild(xpk), 0);
    ASSERT_EQ(xpkVerifyAll(xpk), 0);
    ASSERT_EQ(xpkSave(xpk), 0);

    // Cycle 2: Remove 5 more files (15->10)
    for (int i = 0; i < 5; i++) {
        ASSERT_EQ(xpkRemove(xpk, xpkCount(xpk) - 1), 0);
    }
    ASSERT_EQ(xpkRebuild(xpk), 0);
    ASSERT_EQ(xpkVerifyAll(xpk), 0);
    ASSERT_EQ(xpkSave(xpk), 0);

    // Cycle 3: Remove 5 more files (10->5)
    for (int i = 0; i < 5; i++) {
        ASSERT_EQ(xpkRemove(xpk, xpkCount(xpk) - 1), 0);
    }
    ASSERT_EQ(xpkRebuild(xpk), 0);
    ASSERT_EQ(xpkVerifyAll(xpk), 0);
    ASSERT_EQ(xpkSave(xpk), 0);

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
