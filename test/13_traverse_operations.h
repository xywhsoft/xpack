/*
 * xPack Ver7 - traverse_operations (13)
 */

#include "test_framework.h"

TEST(traverse_basic) {
    xpkObject xpk = xpkOpen("test_13_traverse_basic.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    xpkAppendData(xpk, "Data 1", 6, 6);
    xpkAppendData(xpk, "Data 2", 6, 6);
    xpkAppendData(xpk, "Data 3", 6, 6);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_13_traverse_basic.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkCount(xpk), 3);

    for (uint32_t i = 0; i < xpkCount(xpk); i++) {
        uint32_t size = xpkInfoSize(xpk, i);
        ASSERT_GT(size, 0);
    }

    xpkClose(xpk);
}

TEST(traverse_empty) {
    xpkObject xpk = xpkOpen("test_13_traverse_empty.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkCount(xpk), 0);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_13_traverse_empty.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 0);
    xpkClose(xpk);
}

TEST(traverse_single) {
    xpkObject xpk = xpkOpen("test_13_traverse_single.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    xpkAppendData(xpk, "Single file", 11, 6);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_13_traverse_single.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 1);

    uint32_t size = xpkInfoSize(xpk, 0);
    ASSERT_EQ(size, 11);

    xpkClose(xpk);
}

TEST(traverse_large_count) {
    xpkObject xpk = xpkOpen("test_13_traverse_large.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 100; i++) {
        char data[32];
        sprintf(data, "File %d", i);
        xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_13_traverse_large.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 100);

    for (uint32_t i = 0; i < xpkCount(xpk); i++) {
        ASSERT_GT(xpkInfoSize(xpk, i), 0);
        ASSERT_GT(xpkInfoPacked(xpk, i), 0);
    }

    xpkClose(xpk);
}

TEST(traverse_info_access) {
    xpkObject xpk = xpkOpen("test_13_traverse_info.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    char testData[512];
    memset(testData, 'X', sizeof(testData));

    xpkAppendData(xpk, testData, sizeof(testData), 6);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_13_traverse_info.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkInfoSize(xpk, 0), 512);
    ASSERT_EQ(xpkInfoLevel(xpk, 0), 6);
    ASSERT_GT(xpkInfoPacked(xpk, 0), 0);
    ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_UNKNOWN);

    uint32_t hash = xpkInfoHash(xpk, 0);
    ASSERT_NE(hash, 0);

    xpkClose(xpk);
}

TEST(traverse_order_preservation) {
    xpkObject xpk = xpkOpen("test_13_traverse_order.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    xpkAppendData(xpk, "First", 5, 6);
    xpkAppendData(xpk, "Second", 6, 6);
    xpkAppendData(xpk, "Third", 5, 6);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_13_traverse_order.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 3);

    const char* expected[] = {"First", "Second", "Third"};
    for (uint32_t i = 0; i < 3; i++) {
        uint32_t outSize = 0;
        void* data = xpkExtractData(xpk, i, &outSize);
        ASSERT_NOT_NULL(data);
        ASSERT_EQ(outSize, strlen(expected[i]));
        ASSERT_EQ(memcmp(data, expected[i], outSize), 0);
        xpkFree(data);
    }

    xpkClose(xpk);
}

TEST(traverse_match_case_sensitive_linux) {
    xpkObject xpk = xpkOpen("test_13_traverse_case_linux.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_LINUX), 0);

    xpkPathAppendData(xpk, "files/README.txt", "Readme", 6, 6);
    xpkPathAppendData(xpk, "files/readme.txt", "readme", 6, 6);
    xpkPathAppendData(xpk, "files/README.TXT", "README", 6, 6);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_13_traverse_case_linux.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 3);

    for (uint32_t i = 0; i < xpkCount(xpk); i++) {
        ASSERT_GT(xpkInfoSize(xpk, i), 0);
    }

    xpkClose(xpk);
}

void register_13_traverse_operations_tests(void) {
    TEST_REGISTER(traverse_basic, CAT_TRAVERSE, "Test basic traverse operation");
    TEST_REGISTER(traverse_empty, CAT_TRAVERSE, "Test traverse empty package");
    TEST_REGISTER(traverse_single, CAT_TRAVERSE, "Test traverse single file");
    TEST_REGISTER(traverse_large_count, CAT_TRAVERSE, "Test traverse with many files");
    TEST_REGISTER(traverse_info_access, CAT_TRAVERSE, "Test traverse info access");
    TEST_REGISTER(traverse_order_preservation, CAT_TRAVERSE, "Test traverse order preservation");
    TEST_REGISTER(traverse_match_case_sensitive_linux, CAT_TRAVERSE, "Test Linux case sensitive traverse");
}
