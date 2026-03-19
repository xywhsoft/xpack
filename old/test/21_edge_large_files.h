/*
 * xPack Ver7 - Edge Large Files (21)
 */

#include "test_framework.h"

static char* createTestData_21(int size, char pattern) {
    char* pData = (char*)malloc(size);
    for (int i = 0; i < size; i++) {
        pData[i] = (char)(pattern + (i % 26));
    }
    return pData;
}

TEST(exactly_4mb_file) {
    int iSize = 4 * 1024 * 1024;
    char* pData = createTestData_21(iSize, 'A');
    const char* sFilename = "test_21_4mb.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_NE(xpkAppendData(xpk, pData, iSize, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    uint32_t outSize = 0;
    char* pExtracted = (char*)xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(pExtracted);
    ASSERT_EQ(outSize, (uint32_t)iSize);
    ASSERT_EQ(memcmp(pData, pExtracted, iSize), 0);

    xpkFree(pExtracted);
    free(pData);
    xpkClose(xpk);
}

TEST(just_over_4mb_file) {
    int iSize = 4 * 1024 * 1024 + 1;
    char* pData = createTestData_21(iSize, 'B');
    const char* sFilename = "test_21_4mb_plus1.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_NE(xpkAppendData(xpk, pData, iSize, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    uint32_t outSize = 0;
    char* pExtracted = (char*)xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(pExtracted);
    ASSERT_EQ(outSize, (uint32_t)iSize);
    ASSERT_EQ(memcmp(pData, pExtracted, iSize), 0);

    xpkFree(pExtracted);
    free(pData);
    xpkClose(xpk);
}

TEST(just_under_4mb_file) {
    int iSize = 4 * 1024 * 1024 - 1;
    char* pData = createTestData_21(iSize, 'C');
    const char* sFilename = "test_21_4mb_minus1.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_NE(xpkAppendData(xpk, pData, iSize, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    uint32_t outSize = 0;
    char* pExtracted = (char*)xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(pExtracted);
    ASSERT_EQ(outSize, (uint32_t)iSize);
    ASSERT_EQ(memcmp(pData, pExtracted, iSize), 0);

    xpkFree(pExtracted);
    free(pData);
    xpkClose(xpk);
}

TEST(exactly_1mb_file) {
    int iSize = 1024 * 1024;
    char* pData = createTestData_21(iSize, 'D');
    const char* sFilename = "test_21_1mb.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_NE(xpkAppendData(xpk, pData, iSize, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    uint32_t outSize = 0;
    char* pExtracted = (char*)xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(pExtracted);
    ASSERT_EQ(outSize, (uint32_t)iSize);
    ASSERT_EQ(memcmp(pData, pExtracted, iSize), 0);

    xpkFree(pExtracted);
    free(pData);
    xpkClose(xpk);
}

TEST(exactly_2mb_file) {
    int iSize = 2 * 1024 * 1024;
    char* pData = createTestData_21(iSize, 'E');
    const char* sFilename = "test_21_2mb.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_NE(xpkAppendData(xpk, pData, iSize, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    uint32_t outSize = 0;
    char* pExtracted = (char*)xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(pExtracted);
    ASSERT_EQ(outSize, (uint32_t)iSize);
    ASSERT_EQ(memcmp(pData, pExtracted, iSize), 0);

    xpkFree(pExtracted);
    free(pData);
    xpkClose(xpk);
}

TEST(exactly_10mb_file) {
    int iSize = 10 * 1024 * 1024;
    char* pData = createTestData_21(iSize, 'F');
    const char* sFilename = "test_21_10mb.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_NE(xpkAppendData(xpk, pData, iSize, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    uint32_t outSize = 0;
    char* pExtracted = (char*)xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(pExtracted);
    ASSERT_EQ(outSize, (uint32_t)iSize);
    ASSERT_EQ(memcmp(pData, pExtracted, iSize), 0);

    xpkFree(pExtracted);
    free(pData);
    xpkClose(xpk);
}

TEST(exactly_50mb_file) {
    int iSize = 50 * 1024 * 1024;
    char* pData = createTestData_21(iSize, 'G');
    const char* sFilename = "test_21_50mb.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_NE(xpkAppendData(xpk, pData, iSize, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    uint32_t outSize = 0;
    char* pExtracted = (char*)xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(pExtracted);
    ASSERT_EQ(outSize, (uint32_t)iSize);
    ASSERT_EQ(memcmp(pData, pExtracted, iSize), 0);

    xpkFree(pExtracted);
    free(pData);
    xpkClose(xpk);
}

TEST(exactly_100mb_file) {
    int iSize = 100 * 1024 * 1024;
    char* pData = createTestData_21(iSize, 'H');
    const char* sFilename = "test_21_100mb.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_NE(xpkAppendData(xpk, pData, iSize, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    uint32_t outSize = 0;
    char* pExtracted = (char*)xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(pExtracted);
    ASSERT_EQ(outSize, (uint32_t)iSize);
    ASSERT_EQ(memcmp(pData, pExtracted, iSize), 0);

    xpkFree(pExtracted);
    free(pData);
    xpkClose(xpk);
}

TEST(block_size_boundary_64kb) {
    int iSize = 64 * 1024;
    char* pData = createTestData_21(iSize, 'I');
    const char* sFilename = "test_21_64kb.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_NE(xpkAppendData(xpk, pData, iSize, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    uint32_t outSize = 0;
    char* pExtracted = (char*)xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(pExtracted);
    ASSERT_EQ(outSize, (uint32_t)iSize);
    ASSERT_EQ(memcmp(pData, pExtracted, iSize), 0);

    xpkFree(pExtracted);
    free(pData);
    xpkClose(xpk);
}

TEST(block_size_boundary_256kb) {
    int iSize = 256 * 1024;
    char* pData = createTestData_21(iSize, 'J');
    const char* sFilename = "test_21_256kb.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_NE(xpkAppendData(xpk, pData, iSize, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    uint32_t outSize = 0;
    char* pExtracted = (char*)xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(pExtracted);
    ASSERT_EQ(outSize, (uint32_t)iSize);
    ASSERT_EQ(memcmp(pData, pExtracted, iSize), 0);

    xpkFree(pExtracted);
    free(pData);
    xpkClose(xpk);
}

TEST(block_size_boundary_1mb_plus_1) {
    int iSize = 1024 * 1024 + 1;
    char* pData = createTestData_21(iSize, 'K');
    const char* sFilename = "test_21_1mb_plus1.xpk";

    xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_NE(xpkAppendData(xpk, pData, iSize, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen(sFilename, 0, 0);
    ASSERT_NOT_NULL(xpk);

    uint32_t outSize = 0;
    char* pExtracted = (char*)xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(pExtracted);
    ASSERT_EQ(outSize, (uint32_t)iSize);
    ASSERT_EQ(memcmp(pData, pExtracted, iSize), 0);

    xpkFree(pExtracted);
    free(pData);
    xpkClose(xpk);
}

void register_21_edge_large_files_tests(void) {
    TEST_REGISTER(exactly_4mb_file, CAT_EDGE, "Test exactly 4MB file");
    TEST_REGISTER(just_over_4mb_file, CAT_EDGE, "Test just over 4MB file");
    TEST_REGISTER(just_under_4mb_file, CAT_EDGE, "Test just under 4MB file");
    TEST_REGISTER(exactly_1mb_file, CAT_EDGE, "Test exactly 1MB file");
    TEST_REGISTER(exactly_2mb_file, CAT_EDGE, "Test exactly 2MB file");
    TEST_REGISTER(exactly_10mb_file, CAT_EDGE, "Test exactly 10MB file");
    TEST_REGISTER(exactly_50mb_file, CAT_EDGE, "Test exactly 50MB file");
    TEST_REGISTER(exactly_100mb_file, CAT_EDGE, "Test exactly 100MB file");
    TEST_REGISTER(block_size_boundary_64kb, CAT_EDGE, "Test block size boundary 64KB");
    TEST_REGISTER(block_size_boundary_256kb, CAT_EDGE, "Test block size boundary 256KB");
    TEST_REGISTER(block_size_boundary_1mb_plus_1, CAT_EDGE, "Test block size boundary 1MB+1");
}
