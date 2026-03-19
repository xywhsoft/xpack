/*
 * xPack Ver7 - Edge Many Files (22)
 */

#include "test_framework.h"

static char* createTestData_22(int size, char pattern) {
    char* pData = (char*)malloc(size);
    for (int i = 0; i < size; i++) {
        pData[i] = (char)(pattern + (i % 26));
    }
    return pData;
}

TEST(single_file) {
    char* pData = createTestData_22(1024, 'A');
    const char* sFilename = "test_22_single_file.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_NE(xpkAppendData(xpk, pData, 1024, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkCount(xpk), 1);

    free(pData);
    xpkClose(xpk);
}

TEST(exactly_256_files) {
    const char* sFilename = "test_22_256_files.xpk";

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

    for (int i = 0; i < 256; i++) {
        char* pData = createTestData_22(512, 'A' + (i % 26));
        ASSERT_NE(xpkAppendData(xpk, pData, 512, i), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkCount(xpk), 256);
    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 256);
    xpkClose(xpk);
}

TEST(exactly_65536_files) {
    const char* sFilename = "test_22_65536_files.xpk";

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

    for (int i = 0; i < 65536; i++) {
        char* pData = createTestData_22(256, 'A' + (i % 26));
        ASSERT_NE(xpkAppendData(xpk, pData, 256, i), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkCount(xpk), 65536);
    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 65536);
    xpkClose(xpk);
}

TEST(one_thousand_files) {
    const char* sFilename = "test_22_1000_files.xpk";

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

    for (int i = 0; i < 1000; i++) {
        char* pData = createTestData_22(512, 'A' + (i % 26));
        ASSERT_NE(xpkAppendData(xpk, pData, 512, i), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkCount(xpk), 1000);
    xpkClose(xpk);
}

TEST(ten_thousand_files) {
    const char* sFilename = "test_22_10000_files.xpk";

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

    for (int i = 0; i < 10000; i++) {
        char* pData = createTestData_22(256, 'A' + (i % 26));
        ASSERT_NE(xpkAppendData(xpk, pData, 256, i), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkCount(xpk), 10000);
    xpkClose(xpk);
}

TEST(mixed_size_files) {
    const char* sFilename = "test_22_mixed_sizes.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    int sizes[] = {1, 10, 100, 1024, 10240, 102400, 1048576};
    for (int i = 0; i < 7; i++) {
        char* pData = createTestData_22(sizes[i], 'A' + i);
        ASSERT_NE(xpkAppendData(xpk, pData, sizes[i], i + 1), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkCount(xpk), 7);
    xpkClose(xpk);
}

TEST(files_with_gaps) {
    const char* sFilename = "test_22_gaps.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 10; i++) {
        char* pData = createTestData_22(512, 'A' + i);
        ASSERT_NE(xpkAppendData(xpk, pData, 512, (i + 1) * 10), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkCount(xpk), 10);
    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 10);
    xpkClose(xpk);
}

TEST(sequential_positions) {
    const char* sFilename = "test_22_sequential.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 100; i++) {
        char* pData = createTestData_22(1024, 'A' + (i % 26));
        ASSERT_NE(xpkAppendData(xpk, pData, 1024, i + 1), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkCount(xpk), 100);
    xpkClose(xpk);
}

TEST(random_positions) {
    const char* sFilename = "test_22_random_pos.xpk";
    int positions[20];

    for (int i = 0; i < 20; i++) {
        positions[i] = (i * 17) + 1;
    }

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 20; i++) {
        char* pData = createTestData_22(512, 'A' + (i % 26));
        ASSERT_NE(xpkAppendData(xpk, pData, 512, positions[i]), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkCount(xpk), 20);
    xpkClose(xpk);
}

TEST(files_with_duplicate_data) {
    const char* sFilename = "test_22_duplicate.xpk";
    char* pData = createTestData_22(2048, 'D');

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 50; i++) {
        ASSERT_NE(xpkAppendData(xpk, pData, 2048, i + 1), UINT32_MAX);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkCount(xpk), 50);

    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 50; i++) {
        uint32_t outSize = 0;
        char* pExtracted = (char*)xpkExtractData(xpk, i, &outSize);
        ASSERT_NOT_NULL(pExtracted);
        ASSERT_EQ(outSize, 2048);
        ASSERT_EQ(memcmp(pData, pExtracted, 2048), 0);
        xpkFree(pExtracted);
    }

    free(pData);
    xpkClose(xpk);
}

TEST(remove_middle_file_from_many) {
    const char* sFilename = "test_22_remove_middle.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 20; i++) {
        char* pData = createTestData_22(512, 'A' + (i % 26));
        ASSERT_NE(xpkAppendData(xpk, pData, 512, i + 1), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkCount(xpk), 20);
    ASSERT_EQ(xpkRemove(xpk, 10), 0);
    ASSERT_EQ(xpkCount(xpk), 19);
    ASSERT_EQ(xpkSave(xpk), 0);

    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 19);
    xpkClose(xpk);
}

TEST(update_many_files) {
    const char* sFilename = "test_22_update_many.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 30; i++) {
        char* pData = createTestData_22(512, 'A' + (i % 26));
        ASSERT_NE(xpkAppendData(xpk, pData, 512, i + 1), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);

    for (int i = 0; i < 30; i++) {
        char* pNewData = createTestData_22(1024, 'X' + (i % 26));
        ASSERT_EQ(xpkUpdateData(xpk, i, pNewData, 1024, 6), 0);
        free(pNewData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkCount(xpk), 30);
    xpkClose(xpk);
}

TEST(extract_all_from_many_files) {
    const char* sFilename = "test_22_extract_all.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 50; i++) {
        char* pData = createTestData_22(512, 'A' + (i % 26));
        ASSERT_NE(xpkAppendData(xpk, pData, 512, i + 1), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkExtractAll(xpk, "."), 0);
    ASSERT_EQ(xpkCount(xpk), 50);

    xpkClose(xpk);
}

TEST(verify_all_many_files) {
    const char* sFilename = "test_22_verify_all.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 100; i++) {
        char* pData = createTestData_22(256, 'A' + (i % 26));
        ASSERT_NE(xpkAppendData(xpk, pData, 256, i + 1), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    ASSERT_EQ(xpkVerifyAll(xpk), 0);
    ASSERT_EQ(xpkCount(xpk), 100);

    xpkClose(xpk);
}

TEST(statistics_on_many_files) {
    const char* sFilename = "test_22_stats_many.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 200; i++) {
        char* pData = createTestData_22(512, 'A' + (i % 26));
        ASSERT_NE(xpkAppendData(xpk, pData, 512, i + 1), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkSave(xpk), 0);

    xpkStat stat;
    ASSERT_EQ(xpkStatGet(xpk, &stat), 0);
    ASSERT_EQ(stat.totalSize, 200 * 512);
    ASSERT_NE(stat.packedSize, 0);

    ASSERT_EQ(xpkCount(xpk), 200);
    xpkClose(xpk);
}

void register_22_edge_many_files_tests(void) {
    TEST_REGISTER(single_file, CAT_EDGE, "Test single file");
    TEST_REGISTER(exactly_256_files, CAT_EDGE, "Test exactly 256 files");
    TEST_REGISTER(exactly_65536_files, CAT_EDGE, "Test exactly 65536 files");
    TEST_REGISTER(one_thousand_files, CAT_EDGE, "Test one thousand files");
    TEST_REGISTER(ten_thousand_files, CAT_EDGE, "Test ten thousand files");
    TEST_REGISTER(mixed_size_files, CAT_EDGE, "Test mixed size files");
    TEST_REGISTER(files_with_gaps, CAT_EDGE, "Test files with gaps");
    TEST_REGISTER(sequential_positions, CAT_EDGE, "Test sequential positions");
    TEST_REGISTER(random_positions, CAT_EDGE, "Test random positions");
    TEST_REGISTER(files_with_duplicate_data, CAT_EDGE, "Test files with duplicate data");
    TEST_REGISTER(remove_middle_file_from_many, CAT_EDGE, "Test remove middle file from many");
    TEST_REGISTER(update_many_files, CAT_EDGE, "Test update many files");
    TEST_REGISTER(extract_all_from_many_files, CAT_EDGE, "Test extract all from many files");
    TEST_REGISTER(verify_all_many_files, CAT_EDGE, "Test verify all many files");
    TEST_REGISTER(statistics_on_many_files, CAT_EDGE, "Test statistics on many files");
}
