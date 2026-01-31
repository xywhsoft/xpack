/*
 * xPack Ver7 - Core 模式操作测试 (02)
 */

#include "test_framework.h"

TEST(core_append_extract_file) {
    xpkObject xpk = xpkOpen("test_02_core_file.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    char testData[256];
    memset(testData, 'A', sizeof(testData));
    strcpy(testData, "Hello from file!");

    char srcPath[256];
    sprintf(srcPath, "test_src_02.txt");
    FILE* fp = fopen(srcPath, "wb");
    fwrite(testData, 1, sizeof(testData), fp);
    fclose(fp);

    uint32_t pos = xpkAppendFile(xpk, srcPath, 6);
    ASSERT_EQ(pos, 0);
    ASSERT_EQ(xpkCount(xpk), 1);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_02_core_file.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    char dstPath[256];
    sprintf(dstPath, "test_dst_02.txt");
    ASSERT_EQ(xpkExtractFile(xpk, 0, dstPath), 0);

    fp = fopen(dstPath, "rb");
    char readData[256];
    size_t readSize = fread(readData, 1, sizeof(readData), fp);
    fclose(fp);

    ASSERT_EQ(readSize, sizeof(testData));
    ASSERT_EQ(memcmp(readData, testData, readSize), 0);

    xpkClose(xpk);

    xrtFileDelete(srcPath);
    xrtFileDelete(dstPath);
}

TEST(core_update_file) {
    xpkObject xpk = xpkOpen("test_02_update.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    char data1[100] = "Original data";
    xpkAppendData(xpk, data1, (uint32_t)strlen(data1), 6);

    char srcPath[256];
    sprintf(srcPath, "test_src_update.txt");
    FILE* fp = fopen(srcPath, "wb");
    fwrite("Updated data", 1, 12, fp);
    fclose(fp);

    ASSERT_EQ(xpkUpdateFile(xpk, 0, srcPath, 6), 0);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_02_update.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    uint32_t outSize = 0;
    void* data = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(outSize, 12);
    ASSERT_EQ(memcmp(data, "Updated data", 12), 0);
    xpkFree(data);

    xpkClose(xpk);

    xrtFileDelete(srcPath);
}

TEST(core_update_data) {
    xpkObject xpk = xpkOpen("test_02_update_data.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    char data1[100] = "Original data";
    xpkAppendData(xpk, data1, (uint32_t)strlen(data1), 6);

    char data2[100] = "Updated data content";
    ASSERT_EQ(xpkUpdateData(xpk, 0, data2, (uint32_t)strlen(data2), 6), 0);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_02_update_data.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    uint32_t outSize = 0;
    void* data = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(outSize, strlen(data2));
    ASSERT_EQ(memcmp(data, data2, outSize), 0);
    xpkFree(data);

    xpkClose(xpk);
}

TEST(core_remove_file) {
    xpkObject xpk = xpkOpen("test_02_remove.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    xpkAppendData(xpk, "Data 1", 6, 6);
    xpkAppendData(xpk, "Data 2", 6, 6);
    xpkAppendData(xpk, "Data 3", 6, 6);

    ASSERT_EQ(xpkCount(xpk), 3);

    ASSERT_EQ(xpkRemove(xpk, 1), 0);
    ASSERT_EQ(xpkCount(xpk), 2);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_02_remove.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 2);

    uint32_t outSize = 0;
    void* data = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(memcmp(data, "Data 1", 6), 0);
    xpkFree(data);

    data = xpkExtractData(xpk, 1, &outSize);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(memcmp(data, "Data 3", 6), 0);
    xpkFree(data);

    xpkClose(xpk);
}

TEST(core_remove_first_last) {
    xpkObject xpk = xpkOpen("test_02_remove_first_last.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    xpkAppendData(xpk, "First", 5, 6);
    xpkAppendData(xpk, "Middle", 6, 6);
    xpkAppendData(xpk, "Last", 4, 6);

    ASSERT_EQ(xpkRemove(xpk, 0), 0);
    ASSERT_EQ(xpkCount(xpk), 2);

    ASSERT_EQ(xpkRemove(xpk, 1), 0);
    ASSERT_EQ(xpkCount(xpk), 1);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_02_remove_first_last.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 1);

    uint32_t outSize = 0;
    void* data = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(memcmp(data, "Middle", 6), 0);
    xpkFree(data);

    xpkClose(xpk);
}

TEST(core_info_functions) {
    xpkObject xpk = xpkOpen("test_02_info.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    char testData[1000];
    memset(testData, 'X', sizeof(testData));

    xpkAppendData(xpk, testData, sizeof(testData), 6);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_02_info.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    void* info = xpkInfo(xpk, 0);
    ASSERT_NOT_NULL(info);

    ASSERT_EQ(xpkInfoSize(xpk, 0), sizeof(testData));

    ASSERT_GT(xpkInfoPacked(xpk, 0), 0);
    ASSERT_LT(xpkInfoPacked(xpk, 0), sizeof(testData));

    ASSERT_EQ(xpkInfoLevel(xpk, 0), 6);

    ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_UNKNOWN);

    uint32_t hash1 = xpkInfoHash(xpk, 0);
    ASSERT_NE(hash1, 0);

    uint32_t hash2 = xrtHash32((ptr)testData, sizeof(testData));
    ASSERT_EQ(hash1, hash2);

    xpkClose(xpk);
}

TEST(core_info_type_set) {
    xpkObject xpk = xpkOpen("test_02_info_type.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    xpkAppendData(xpk, "Binary data", 11, 6);
    xpkAppendData(xpk, "Text data", 9, 6);

    ASSERT_EQ(xpkInfoTypeSet(xpk, 0, XPK_FTYPE_BINARY), 0);
    ASSERT_EQ(xpkInfoTypeSet(xpk, 1, XPK_FTYPE_TEXT), 0);
    ASSERT_EQ(xpkInfoTypeSet(xpk, 1, XPK_FTYPE_ARCHIVE), 0);

    ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_BINARY);
    ASSERT_EQ(xpkInfoType(xpk, 1), XPK_FTYPE_ARCHIVE);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_02_info_type.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_BINARY);
    ASSERT_EQ(xpkInfoType(xpk, 1), XPK_FTYPE_ARCHIVE);

    xpkClose(xpk);
}

TEST(core_readonly_protection) {
    xpkObject xpk = xpkOpen("test_02_readonly.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    xpkAppendData(xpk, "Data", 4, 6);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_02_readonly.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    uint32_t pos = xpkAppendData(xpk, "New", 3, 6);
    ASSERT_EQ(pos, UINT32_MAX);
    ASSERT_NE(xpkLastError(), 0);

    ASSERT_NE(xpkUpdateData(xpk, 0, "Updated", 7, 6), 0);
    ASSERT_NE(xpkLastError(), 0);

    ASSERT_NE(xpkRemove(xpk, 0), 0);
    ASSERT_NE(xpkLastError(), 0);

    ASSERT_NE(xpkInfoTypeSet(xpk, 0, XPK_FTYPE_TEXT), 0);
    ASSERT_NE(xpkLastError(), 0);

    uint32_t outSize = 0;
    void* data = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(outSize, 4);
    ASSERT_EQ(memcmp(data, "Data", 4), 0);
    xpkFree(data);

    xpkClose(xpk);
}

TEST(core_multiple_updates) {
    xpkObject xpk = xpkOpen("test_02_multi_update.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 10; i++) {
        xpkAppendData(xpk, "Initial", 8, 6);
    }

    ASSERT_EQ(xpkCount(xpk), 10);

    for (int i = 0; i < 10; i++) {
        char newData[64];
        sprintf(newData, "Update %d", i);
        ASSERT_EQ(xpkUpdateData(xpk, i, newData, (uint32_t)strlen(newData), 6), 0);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_02_multi_update.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 10);

    for (int i = 0; i < 10; i++) {
        char expected[64];
        sprintf(expected, "Update %d", i);

        uint32_t outSize = 0;
        void* data = xpkExtractData(xpk, i, &outSize);
        ASSERT_NOT_NULL(data);
        ASSERT_EQ(memcmp(data, expected, strlen(expected)), 0);
        xpkFree(data);
    }

    xpkClose(xpk);
}

TEST(core_append_after_remove) {
    xpkObject xpk = xpkOpen("test_02_append_after_remove.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    xpkAppendData(xpk, "File 1", 6, 6);
    xpkAppendData(xpk, "File 2", 6, 6);
    xpkAppendData(xpk, "File 3", 6, 6);

    ASSERT_EQ(xpkRemove(xpk, 1), 0);

    uint32_t pos = xpkAppendData(xpk, "File 4", 6, 6);
    ASSERT_EQ(pos, 2);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_02_append_after_remove.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 3);

    uint32_t outSize = 0;
    void* data = xpkExtractData(xpk, 2, &outSize);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(memcmp(data, "File 4", 6), 0);
    xpkFree(data);

    xpkClose(xpk);
}

TEST(core_all_levels) {
    for (int level = 0; level <= 15; level++) {
        char filename[64];
        sprintf(filename, "test_02_level_%d.xpk", level);

        xpkObject xpk = xpkOpen(filename, 0, 0);
        ASSERT_NOT_NULL(xpk);

        char data[512];
        memset(data, 'A' + (level % 26), sizeof(data));

        uint32_t pos = xpkAppendData(xpk, data, sizeof(data), level);
        ASSERT_EQ(pos, 0);

        ASSERT_EQ(xpkInfoLevel(xpk, 0), level);

        ASSERT_EQ(xpkSave(xpk), 0);
        xpkClose(xpk);

        xpk = xpkOpen(filename, 0, 1);
        ASSERT_NOT_NULL(xpk);

        uint32_t outSize = 0;
        void* extracted = xpkExtractData(xpk, 0, &outSize);
        ASSERT_NOT_NULL(extracted);
        ASSERT_EQ(outSize, sizeof(data));
        ASSERT_EQ(memcmp(extracted, data, outSize), 0);
        xpkFree(extracted);

        xpkClose(xpk);
    }
}

void register_02_core_operations_tests(void) {
    TEST_REGISTER(core_append_extract_file, CAT_CORE, "Test append and extract file");
    TEST_REGISTER(core_update_file, CAT_CORE, "Test update file");
    TEST_REGISTER(core_update_data, CAT_CORE, "Test update data");
    TEST_REGISTER(core_remove_file, CAT_CORE, "Test remove file");
    TEST_REGISTER(core_remove_first_last, CAT_CORE, "Test remove first and last");
    TEST_REGISTER(core_info_functions, CAT_CORE, "Test info functions");
    TEST_REGISTER(core_info_type_set, CAT_CORE, "Test info type set");
    TEST_REGISTER(core_readonly_protection, CAT_CORE, "Test readonly protection");
    TEST_REGISTER(core_multiple_updates, CAT_CORE, "Test multiple updates");
    TEST_REGISTER(core_append_after_remove, CAT_CORE, "Test append after remove");
    TEST_REGISTER(core_all_levels, CAT_CORE, "Test all compression levels");
}
