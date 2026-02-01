/*
 * xPack Ver7 - Memory Management (26)
 */

#include "test_framework.h"

static char* createTestData_26(int size, char pattern) {
    char* pData = (char*)malloc(size);
    for (int i = 0; i < size; i++) {
        pData[i] = (char)(pattern + (i % 26));
    }
    return pData;
}

TEST(create_and_destroy_package) {
    char* pData = createTestData_26(1024, 'A');
    const char* sFilename = "test_26_create.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_NE(xpkAppendData(xpk, pData, 1024, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    free(pData);
}

TEST(multiple_open_close_cycles) {
    const char* sFilename = "test_26_open_close.xpk";

    for (int i = 0; i < 20; i++) {
        xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
        ASSERT_NOT_NULL(xpk);
        xpkClose(xpk);
    }
}

TEST(extract_with_allocated_buffer) {
    char* pData = createTestData_26(2048, 'B');
    const char* sFilename = "test_26_alloc_buf.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_NE(xpkAppendData(xpk, pData, 2048, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);

    uint32_t outSize = 0;
    void* pExtracted = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(pExtracted);
    ASSERT_EQ(outSize, 2048);
    ASSERT_EQ(memcmp(pData, pExtracted, 2048), 0);
    xpkFree(pExtracted);

    free(pData);
    xpkClose(xpk);
}

TEST(extract_with_null_buffer) {
    char* pData = createTestData_26(1024, 'C');
    const char* sFilename = "test_26_null_buf.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_NE(xpkAppendData(xpk, pData, 1024, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);

    uint32_t outSize = 0;
    void* pExtracted = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(pExtracted);
    ASSERT_EQ(outSize, 1024);
    xpkFree(pExtracted);

    free(pData);
    xpkClose(xpk);
}

TEST(extract_with_insufficient_buffer) {
    char* pData = createTestData_26(2048, 'D');
    const char* sFilename = "test_26_small_buf.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_NE(xpkAppendData(xpk, pData, 2048, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);

    uint32_t outSize = 512;
    void* pExtracted = xpkExtractData(xpk, 0, &outSize);
    if (pExtracted) {
        xpkFree(pExtracted);
    }

    free(pData);
    xpkClose(xpk);
}

TEST(info_with_null_parameters) {
    char* pData = createTestData_26(1024, 'E');
    const char* sFilename = "test_26_null_info.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_NE(xpkAppendData(xpk, pData, 1024, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);

    void* pInfo = xpkInfo(xpk, 0);
    ASSERT_NOT_NULL(pInfo);

    free(pData);
    xpkClose(xpk);
}

TEST(stat_with_null_parameters) {
    char* pData = createTestData_26(2048, 'F');
    const char* sFilename = "test_26_null_stat.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_NE(xpkAppendData(xpk, pData, 2048, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);

    xpkStat stat;
    ASSERT_EQ(xpkStatGet(xpk, &stat), 0);

    free(pData);
    xpkClose(xpk);
}

TEST(user_data_memory_handling) {
    char* pData = createTestData_26(1024, 'G');
    const char* sFilename = "test_26_userdata.xpk";

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);
    ASSERT_NE(xpkAppendData(xpk, pData, 1024, 1), UINT32_MAX);
    ASSERT_EQ(xpkIndexUserDataSet(xpk, 10, 256), 0);
    ASSERT_EQ(xpkSave(xpk), 0);

    int32_t userData = xpkIndexUserData(xpk, 10);
    ASSERT_EQ(userData, 256);

    free(pData);
    xpkClose(xpk);
}

TEST(repeated_append_free_cycles) {
    const char* sFilename = "test_26_append_free.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 100; i++) {
        char* pData = createTestData_26(512, 'A' + (i % 26));
        ASSERT_NE(xpkAppendData(xpk, pData, 512, i + 1), UINT32_MAX);
        free(pData);
    }

    ASSERT_EQ(xpkCount(xpk), 100);
    ASSERT_EQ(xpkSave(xpk), 0);

    xpkClose(xpk);
}

TEST(repeated_extract_free_cycles) {
    const char* sFilename = "test_26_extract_free.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    char* pData = createTestData_26(1024, 'I');
    ASSERT_NE(xpkAppendData(xpk, pData, 1024, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);

    for (int i = 0; i < 50; i++) {
        uint32_t outSize = 0;
        void* pExtracted = xpkExtractData(xpk, 0, &outSize);
        ASSERT_NOT_NULL(pExtracted);
        ASSERT_EQ(outSize, 1024);
        ASSERT_EQ(memcmp(pData, pExtracted, 1024), 0);
        xpkFree(pExtracted);
    }

    free(pData);
    xpkClose(xpk);
}

TEST(find_with_null_result) {
    const char* sFilename = "test_26_null_find.xpk";

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

    char* pData = createTestData_26(1024, 'J');
    ASSERT_NE(xpkAppendData(xpk, pData, 1024, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);

    ASSERT_NE(xpkIndexFind(xpk, 100), UINT32_MAX);
    ASSERT_EQ(xpkIndexFind(xpk, 999), UINT32_MAX);

    free(pData);
    xpkClose(xpk);
}

TEST(each_with_null_callback) {
    const char* sFilename = "test_26_null_each.xpk";

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

    char* pData = createTestData_26(512, 'K');
    for (int i = 0; i < 10; i++) {
        ASSERT_NE(xpkAppendData(xpk, pData, 512, i * 10), UINT32_MAX);
    }
    ASSERT_EQ(xpkSave(xpk), 0);

    int iCount = 0;
    ASSERT_EQ(xpkEach(xpk, NULL, &iCount), 0);
    ASSERT_EQ(iCount, 10);

    free(pData);
    xpkClose(xpk);
}

TEST(match_with_null_callback) {
    const char* sFilename = "test_26_null_match.xpk";

    xpkObject xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_LINUX), 0);

    char* pData = createTestData_26(512, 'L');
    uint32_t pos1 = xpkPathAppendData(xpk, "test/file1.txt", pData, 512, 1);
    ASSERT_NE(pos1, UINT32_MAX);
    uint32_t pos2 = xpkPathAppendData(xpk, "test/file2.txt", pData, 512, 1);
    ASSERT_NE(pos2, UINT32_MAX);
    uint32_t pos3 = xpkPathAppendData(xpk, "other/file3.txt", pData, 512, 1);
    ASSERT_NE(pos3, UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);

    int iCount = 0;
    ASSERT_EQ(xpkEachMatch(xpk, "test/*.txt", NULL, &iCount), 0);
    ASSERT_EQ(iCount, 2);

    free(pData);
    xpkClose(xpk);
}

TEST(head_with_null_buffer) {
    const char* sFilename = "test_26_null_head.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    char* pData = createTestData_26(2048, 'M');
    ASSERT_NE(xpkAppendData(xpk, pData, 2048, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);

    xpkHead* pHead = xpkGetHead(xpk);
    ASSERT_NOT_NULL(pHead);

    free(pData);
    xpkClose(xpk);
}

TEST(type_and_disc_code_queries) {
    const char* sFilename = "test_26_queries.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkType(xpk), XPK_TYPE_CORE);
    ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);
    ASSERT_EQ(xpkType(xpk), XPK_TYPE_INDEX);

    ASSERT_EQ(xpkDiscCodeSet(xpk, 0x12345678), 0);
    ASSERT_EQ(xpkDiscCode(xpk), 0x12345678);

    xpkClose(xpk);
}

void register_26_memory_management_tests(void) {
    TEST_REGISTER(create_and_destroy_package, CAT_CORE, "Create and destroy package");
    TEST_REGISTER(multiple_open_close_cycles, CAT_CORE, "Multiple open close cycles");
    TEST_REGISTER(extract_with_allocated_buffer, CAT_CORE, "Extract with allocated buffer");
    TEST_REGISTER(extract_with_null_buffer, CAT_CORE, "Extract with null buffer");
    TEST_REGISTER(extract_with_insufficient_buffer, CAT_CORE, "Extract with insufficient buffer");
    TEST_REGISTER(info_with_null_parameters, CAT_CORE, "Info with null parameters");
    TEST_REGISTER(stat_with_null_parameters, CAT_CORE, "Stat with null parameters");
    TEST_REGISTER(user_data_memory_handling, CAT_CORE, "User data memory handling");
    TEST_REGISTER(repeated_append_free_cycles, CAT_CORE, "Repeated append free cycles");
    TEST_REGISTER(repeated_extract_free_cycles, CAT_CORE, "Repeated extract free cycles");
    TEST_REGISTER(find_with_null_result, CAT_CORE, "Find with null result");
    TEST_REGISTER(each_with_null_callback, CAT_CORE, "Each with null callback");
    TEST_REGISTER(match_with_null_callback, CAT_CORE, "Match with null callback");
    TEST_REGISTER(head_with_null_buffer, CAT_CORE, "Head with null buffer");
    TEST_REGISTER(type_and_disc_code_queries, CAT_CORE, "Type and disc code queries");
}
