/*
 * xPack Ver7 - Core 模式边界情况测试 (03)
 */

#include "test_framework.h"

TEST(core_empty_data) {
    xpkObject xpk = xpkOpen("test_03_empty_data.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    uint32_t pos = xpkAppendData(xpk, NULL, 0, 6);
    ASSERT_EQ(pos, 0);
    ASSERT_EQ(xpkCount(xpk), 1);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_03_empty_data.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    uint32_t outSize = 0;
    void* data = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(outSize, 0);
    xpkFree(data);

    xpkClose(xpk);
}

TEST(core_one_byte_data) {
    xpkObject xpk = xpkOpen("test_03_one_byte.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    uint8_t byte = 0x42;
    uint32_t pos = xpkAppendData(xpk, &byte, 1, 6);
    ASSERT_EQ(pos, 0);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_03_one_byte.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    uint32_t outSize = 0;
    uint8_t* data = (uint8_t*)xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(outSize, 1);
    ASSERT_EQ(data[0], 0x42);
    xpkFree(data);

    xpkClose(xpk);
}

TEST(core_large_data_1mb) {
    xpkObject xpk = xpkOpen("test_03_1mb.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    size_t size = 1024 * 1024;
    void* data = malloc(size);
    ASSERT_NOT_NULL(data);
    memset(data, 'A', size);

    uint32_t pos = xpkAppendData(xpk, data, (uint32_t)size, 6);
    ASSERT_EQ(pos, 0);
    ASSERT_EQ(xpkInfoSize(xpk, 0), (uint32_t)size);

    free(data);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_03_1mb.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    uint32_t outSize = 0;
    void* extracted = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(extracted);
    ASSERT_EQ(outSize, (uint32_t)size);

    uint8_t* ptr = (uint8_t*)extracted;
    for (size_t i = 0; i < size; i++) {
        ASSERT_EQ(ptr[i], 'A');
    }

    xpkFree(extracted);
    xpkClose(xpk);
}

TEST(core_invalid_position) {
    xpkObject xpk = xpkOpen("test_03_invalid_pos.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    xpkAppendData(xpk, "Data", 4, 6);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_03_invalid_pos.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    uint32_t outSize = 0;
    void* data = xpkExtractData(xpk, 1, &outSize);
    ASSERT_NULL(data);
    ASSERT_NE(xpkLastError(), 0);

    data = xpkExtractData(xpk, 100, &outSize);
    ASSERT_NULL(data);
    ASSERT_NE(xpkLastError(), 0);

    data = xpkExtractData(xpk, UINT32_MAX, &outSize);
    ASSERT_NULL(data);
    ASSERT_NE(xpkLastError(), 0);

    ASSERT_EQ(xpkInfo(xpk, 1), NULL);

    ASSERT_NE(xpkRemove(xpk, 1), 0);
    ASSERT_NE(xpkLastError(), 0);

    ASSERT_NE(xpkUpdateData(xpk, 1, "New", 3, 6), 0);
    ASSERT_NE(xpkLastError(), 0);

    xpkClose(xpk);
}

TEST(core_null_parameters) {
    xpkObject xpk = xpkOpen("test_03_null_param.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    xpkAppendData(xpk, "Data", 4, 6);

    ASSERT_EQ(xpkExtractData(NULL, 0, NULL), NULL);
    ASSERT_EQ(xpkExtractData(xpk, 0, NULL), NULL);

    ASSERT_EQ(xpkExtractFile(NULL, 0, "path.txt"), -1);
    ASSERT_EQ(xpkExtractFile(xpk, 0, NULL), -1);

    ASSERT_EQ(xpkAppendFile(NULL, "file.txt", 6), UINT32_MAX);
    ASSERT_EQ(xpkAppendFile(xpk, NULL, 6), UINT32_MAX);

    ASSERT_EQ(xpkUpdateFile(NULL, 0, "file.txt", 6), -1);
    ASSERT_EQ(xpkUpdateFile(xpk, 0, NULL, 6), -1);

    ASSERT_EQ(xpkUpdateData(NULL, 0, "data", 4, 6), -1);
    ASSERT_EQ(xpkUpdateData(xpk, 0, NULL, 4, 6), -1);

    ASSERT_EQ(xpkRemove(NULL, 0), -1);

    ASSERT_EQ(xpkInfo(NULL, 0), NULL);

    ASSERT_EQ(xpkInfoSize(NULL, 0), 0);
    ASSERT_EQ(xpkInfoPacked(NULL, 0), 0);
    ASSERT_EQ(xpkInfoHash(NULL, 0), 0);
    ASSERT_EQ(xpkInfoLevel(NULL, 0), -1);
    ASSERT_EQ(xpkInfoType(NULL, 0), -1);

    ASSERT_EQ(xpkSave(NULL), -1);

    xpkClose(xpk);
}

TEST(core_multiple_empty_files) {
    xpkObject xpk = xpkOpen("test_03_multi_empty.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    for (int i = 0; i < 10; i++) {
        uint32_t pos = xpkAppendData(xpk, NULL, 0, 6);
        ASSERT_EQ(pos, i);
    }

    ASSERT_EQ(xpkCount(xpk), 10);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_03_multi_empty.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 10);

    for (int i = 0; i < 10; i++) {
        uint32_t outSize = 0;
        void* data = xpkExtractData(xpk, i, &outSize);
        ASSERT_NOT_NULL(data);
        ASSERT_EQ(outSize, 0);
        xpkFree(data);
    }

    xpkClose(xpk);
}

TEST(core_update_empty_to_data) {
    xpkObject xpk = xpkOpen("test_03_update_empty.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    xpkAppendData(xpk, NULL, 0, 6);

    ASSERT_EQ(xpkUpdateData(xpk, 0, "New data", 8, 6), 0);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_03_update_empty.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkInfoSize(xpk, 0), 8);

    uint32_t outSize = 0;
    void* data = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(outSize, 8);
    ASSERT_EQ(memcmp(data, "New data", 8), 0);
    xpkFree(data);

    xpkClose(xpk);
}

TEST(core_update_data_to_empty) {
    xpkObject xpk = xpkOpen("test_03_update_to_empty.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    xpkAppendData(xpk, "Original data", 13, 6);

    ASSERT_EQ(xpkUpdateData(xpk, 0, NULL, 0, 6), 0);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_03_update_to_empty.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkInfoSize(xpk, 0), 0);

    uint32_t outSize = 0;
    void* data = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(outSize, 0);
    xpkFree(data);

    xpkClose(xpk);
}

TEST(core_compress_level_bounds) {
    xpkObject xpk = xpkOpen("test_03_level_bounds.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    char data[512];
    memset(data, 'A', sizeof(data));

    for (int level = -5; level <= 20; level++) {
        uint32_t pos = xpkAppendData(xpk, data, sizeof(data), level);
        ASSERT_NE(pos, UINT32_MAX);

        int actualLevel = xpkInfoLevel(xpk, pos);
        ASSERT_GE(actualLevel, 0);
        ASSERT_LE(actualLevel, 15);

        uint32_t outSize = 0;
        void* extracted = xpkExtractData(xpk, pos, &outSize);
        ASSERT_NOT_NULL(extracted);
        ASSERT_EQ(outSize, sizeof(data));
        ASSERT_EQ(memcmp(extracted, data, outSize), 0);
        xpkFree(extracted);
    }

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);
}

TEST(core_single_file_operations) {
    xpkObject xpk = xpkOpen("test_03_single.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    xpkAppendData(xpk, "Single file", 11, 6);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_03_single.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 1);

    ASSERT_NE(xpkRemove(xpk, 0), 0);
    ASSERT_EQ(xpkCount(xpk), 0);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_03_single.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 0);
    xpkClose(xpk);
}

TEST(reopen_many_times) {
    char filename[64];
    strcpy(filename, "test_03_reopen.xpk");

    for (int cycle = 0; cycle < 5; cycle++) {
        xpkObject xpk = xpkOpen(filename, 0, cycle == 0 ? 0 : 1);
        ASSERT_NOT_NULL(xpk);

        if (cycle == 0) {
            xpkAppendData(xpk, "Data 1", 6, 6);
            xpkAppendData(xpk, "Data 2", 6, 6);
            xpkAppendData(xpk, "Data 3", 6, 6);
        }

        ASSERT_EQ(xpkCount(xpk), 3);

        for (int i = 0; i < 3; i++) {
            uint32_t outSize = 0;
            void* data = xpkExtractData(xpk, i, &outSize);
            ASSERT_NOT_NULL(data);
            ASSERT_EQ(outSize, 6);
            xpkFree(data);
        }

        xpkClose(xpk);
    }
}

TEST(core_all_zeroes_data) {
    xpkObject xpk = xpkOpen("test_03_zeroes.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    size_t size = 10000;
    void* data = malloc(size);
    ASSERT_NOT_NULL(data);
    memset(data, 0, size);

    uint32_t pos = xpkAppendData(xpk, data, (uint32_t)size, 6);
    ASSERT_EQ(pos, 0);

    free(data);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_03_zeroes.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    uint32_t outSize = 0;
    void* extracted = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(extracted);
    ASSERT_EQ(outSize, (uint32_t)size);

    uint8_t* ptr = (uint8_t*)extracted;
    for (size_t i = 0; i < size; i++) {
        ASSERT_EQ(ptr[i], 0);
    }

    xpkFree(extracted);
    xpkClose(xpk);
}

TEST(core_all_ones_data) {
    xpkObject xpk = xpkOpen("test_03_ones.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    size_t size = 10000;
    void* data = malloc(size);
    ASSERT_NOT_NULL(data);
    memset(data, 0xFF, size);

    uint32_t pos = xpkAppendData(xpk, data, (uint32_t)size, 6);
    ASSERT_EQ(pos, 0);

    free(data);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_03_ones.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    uint32_t outSize = 0;
    void* extracted = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(extracted);
    ASSERT_EQ(outSize, (uint32_t)size);

    uint8_t* ptr = (uint8_t*)extracted;
    for (size_t i = 0; i < size; i++) {
        ASSERT_EQ(ptr[i], 0xFF);
    }

    xpkFree(extracted);
    xpkClose(xpk);
}

void register_03_core_edge_cases_tests(void) {
    TEST_REGISTER(core_empty_data, CAT_CORE, "Test empty data");
    TEST_REGISTER(core_one_byte_data, CAT_CORE, "Test one byte data");
    TEST_REGISTER(core_large_data_1mb, CAT_CORE, "Test 1MB data");
    TEST_REGISTER(core_invalid_position, CAT_CORE, "Test invalid position");
    TEST_REGISTER(core_null_parameters, CAT_CORE, "Test null parameters");
    TEST_REGISTER(core_multiple_empty_files, CAT_CORE, "Test multiple empty files");
    TEST_REGISTER(core_update_empty_to_data, CAT_CORE, "Test update empty to data");
    TEST_REGISTER(core_update_data_to_empty, CAT_CORE, "Test update data to empty");
    TEST_REGISTER(core_compress_level_bounds, CAT_CORE, "Test compress level bounds");
    TEST_REGISTER(core_single_file_operations, CAT_CORE, "Test single file operations");
    TEST_REGISTER(reopen_many_times, CAT_CORE, "Test reopen many times");
    TEST_REGISTER(core_all_zeroes_data, CAT_CORE, "Test all zeroes data");
    TEST_REGISTER(core_all_ones_data, CAT_CORE, "Test all ones data");
}
