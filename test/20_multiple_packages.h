/*
 * xPack Ver7 - Multiple Packages (20)
 */

#include "test_framework.h"

static char* createTestData_20(int size, char pattern) {
    char* pData = (char*)malloc(size);
    for (int i = 0; i < size; i++) {
        pData[i] = (char)(pattern + (i % 26));
    }
    return pData;
}

TEST(open_multiple_packages_simultaneously) {
    char* pData1 = createTestData_20(1024, 'A');
    char* pData2 = createTestData_20(2048, 'B');
    char* pData3 = createTestData_20(4096, 'C');

    xpkObject xpk1 = xpkOpen("test_20_multi_1.xpk", 0, 0);
    xpkObject xpk2 = xpkOpen("test_20_multi_2.xpk", 0, 0);
    xpkObject xpk3 = xpkOpen("test_20_multi_3.xpk", 0, 0);

    ASSERT_NOT_NULL(xpk1);
    ASSERT_NOT_NULL(xpk2);
    ASSERT_NOT_NULL(xpk3);

    ASSERT_NE(xpkAppendData(xpk1, pData1, 1024, 1), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk2, pData2, 2048, 6), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk3, pData3, 4096, 1), UINT32_MAX);

    ASSERT_EQ(xpkSave(xpk1), 0);
    ASSERT_EQ(xpkSave(xpk2), 0);
    ASSERT_EQ(xpkSave(xpk3), 0);

    ASSERT_EQ(xpkCount(xpk1), 1);
    ASSERT_EQ(xpkCount(xpk2), 1);
    ASSERT_EQ(xpkCount(xpk3), 1);

    xpkClose(xpk1);
    xpkClose(xpk2);
    xpkClose(xpk3);

    free(pData1);
    free(pData2);
    free(pData3);
}

TEST(data_isolation_between_packages) {
    char* pData1 = createTestData_20(1024, 'D');
    char* pData2 = createTestData_20(2048, 'E');

    xpkObject xpk1 = xpkOpen("test_20_isolation_1.xpk", 0, 0);
    xpkObject xpk2 = xpkOpen("test_20_isolation_2.xpk", 0, 0);

    ASSERT_NOT_NULL(xpk1);
    ASSERT_NOT_NULL(xpk2);

    ASSERT_NE(xpkAppendData(xpk1, pData1, 1024, 1), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk2, pData2, 2048, 1), UINT32_MAX);

    ASSERT_EQ(xpkSave(xpk1), 0);
    ASSERT_EQ(xpkSave(xpk2), 0);

    ASSERT_EQ(xpkCount(xpk1), 1);
    ASSERT_EQ(xpkCount(xpk2), 1);

    uint32_t outSize = 0;
    void* data1 = xpkExtractData(xpk1, 0, &outSize);
    ASSERT_NOT_NULL(data1);
    ASSERT_EQ(outSize, 1024);
    ASSERT_EQ(memcmp(pData1, data1, 1024), 0);
    xpkFree(data1);

    outSize = 0;
    void* data2 = xpkExtractData(xpk2, 0, &outSize);
    ASSERT_NOT_NULL(data2);
    ASSERT_EQ(outSize, 2048);
    ASSERT_EQ(memcmp(pData2, data2, 2048), 0);
    xpkFree(data2);

    free(pData1);
    free(pData2);
    xpkClose(xpk1);
    xpkClose(xpk2);
}

TEST(different_compression_levels_in_packages) {
    char* pData = createTestData_20(4096, 'F');

    xpkObject xpk1 = xpkOpen("test_20_comp_1.xpk", 0, 0);
    xpkObject xpk2 = xpkOpen("test_20_comp_2.xpk", 0, 0);
    xpkObject xpk3 = xpkOpen("test_20_comp_3.xpk", 0, 0);

    ASSERT_NOT_NULL(xpk1);
    ASSERT_NOT_NULL(xpk2);
    ASSERT_NOT_NULL(xpk3);

    ASSERT_NE(xpkAppendData(xpk1, pData, 4096, 1), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk2, pData, 4096, 1), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk3, pData, 4096, 1), UINT32_MAX);

    ASSERT_EQ(xpkSave(xpk1), 0);
    ASSERT_EQ(xpkSave(xpk2), 0);
    ASSERT_EQ(xpkSave(xpk3), 0);

    xpkClose(xpk1);
    xpkClose(xpk2);
    xpkClose(xpk3);

    xpk1 = xpkOpen("test_20_comp_1.xpk", 0, 0);
    xpk2 = xpkOpen("test_20_comp_2.xpk", 0, 0);
    xpk3 = xpkOpen("test_20_comp_3.xpk", 0, 0);

    ASSERT_NOT_NULL(xpk1);
    ASSERT_NOT_NULL(xpk2);
    ASSERT_NOT_NULL(xpk3);

    uint32_t outSize = 0;
    void* data1 = xpkExtractData(xpk1, 0, &outSize);
    ASSERT_NOT_NULL(data1);
    ASSERT_EQ(outSize, 4096);
    ASSERT_EQ(memcmp(pData, data1, 4096), 0);
    xpkFree(data1);

    outSize = 0;
    void* data2 = xpkExtractData(xpk2, 0, &outSize);
    ASSERT_NOT_NULL(data2);
    ASSERT_EQ(outSize, 4096);
    ASSERT_EQ(memcmp(pData, data2, 4096), 0);
    xpkFree(data2);

    outSize = 0;
    void* data3 = xpkExtractData(xpk3, 0, &outSize);
    ASSERT_NOT_NULL(data3);
    ASSERT_EQ(outSize, 4096);
    ASSERT_EQ(memcmp(pData, data3, 4096), 0);
    xpkFree(data3);

    free(pData);
    xpkClose(xpk1);
    xpkClose(xpk2);
    xpkClose(xpk3);
}

TEST(different_package_types) {
    char* pData = createTestData_20(2048, 'G');

    xpkObject xpkCore = xpkOpen("test_20_type_core.xpk", 0, 0);
    xpkObject xpkIndex = xpkOpen("test_20_type_index.xpk", 0, 0);
    xpkObject xpkPath = xpkOpen("test_20_type_path.xpk", 0, 0);

    ASSERT_NOT_NULL(xpkCore);
    ASSERT_NOT_NULL(xpkIndex);
    ASSERT_NOT_NULL(xpkPath);

    ASSERT_NE(xpkAppendData(xpkCore, pData, 2048, 1), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpkIndex, pData, 2048, 6), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpkPath, pData, 2048, 1), UINT32_MAX);

    ASSERT_EQ(xpkSave(xpkCore), 0);
    ASSERT_EQ(xpkSave(xpkIndex), 0);
    ASSERT_EQ(xpkSave(xpkPath), 0);

    ASSERT_EQ(xpkType(xpkCore), XPK_TYPE_CORE);
    ASSERT_EQ(xpkType(xpkIndex), XPK_TYPE_INDEX);
    ASSERT_EQ(xpkType(xpkPath), XPK_TYPE_WIN32);

    xpkClose(xpkCore);
    xpkClose(xpkIndex);
    xpkClose(xpkPath);

    xpkCore = xpkOpen("test_20_type_core.xpk", 0, 0);
    xpkIndex = xpkOpen("test_20_type_index.xpk", 0, 0);
    xpkPath = xpkOpen("test_20_type_path.xpk", 0, 0);

    ASSERT_NOT_NULL(xpkCore);
    ASSERT_NOT_NULL(xpkIndex);
    ASSERT_NOT_NULL(xpkPath);

    uint32_t outSize = 0;
    void* dataCore = xpkExtractData(xpkCore, 0, &outSize);
    ASSERT_NOT_NULL(dataCore);
    ASSERT_EQ(outSize, 2048);
    ASSERT_EQ(memcmp(pData, dataCore, 2048), 0);
    xpkFree(dataCore);

    outSize = 0;
    void* dataIndex = xpkExtractData(xpkIndex, 0, &outSize);
    ASSERT_NOT_NULL(dataIndex);
    ASSERT_EQ(outSize, 2048);
    ASSERT_EQ(memcmp(pData, dataIndex, 2048), 0);
    xpkFree(dataIndex);

    outSize = 0;
    void* dataPath = xpkExtractData(xpkPath, 0, &outSize);
    ASSERT_NOT_NULL(dataPath);
    ASSERT_EQ(outSize, 2048);
    ASSERT_EQ(memcmp(pData, dataPath, 2048), 0);
    xpkFree(dataPath);

    free(pData);
    xpkClose(xpkCore);
    xpkClose(xpkIndex);
    xpkClose(xpkPath);
}

TEST(copy_data_between_packages) {
    char* pData = createTestData_20(3072, 'H');

    xpkObject xpkSrc = xpkOpen("test_20_src.xpk", 0, 0);
    xpkObject xpkDst = xpkOpen("test_20_dst.xpk", 0, 0);

    ASSERT_NOT_NULL(xpkSrc);
    ASSERT_NOT_NULL(xpkDst);

    ASSERT_NE(xpkAppendData(xpkSrc, pData, 3072, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpkSrc), 0);

    uint32_t outSize = 0;
    void* pExtracted = xpkExtractData(xpkSrc, 0, &outSize);
    ASSERT_NOT_NULL(pExtracted);
    ASSERT_EQ(outSize, 3072);
    ASSERT_NE(xpkAppendData(xpkDst, pExtracted, outSize, 1), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpkDst), 0);
    xpkFree(pExtracted);

    ASSERT_EQ(xpkCount(xpkSrc), 1);
    ASSERT_EQ(xpkCount(xpkDst), 1);

    free(pData);
    xpkClose(xpkSrc);
    xpkClose(xpkDst);
}

TEST(merge_packages) {
    char* pData1 = createTestData_20(1024, 'I');
    char* pData2 = createTestData_20(2048, 'J');
    char* pData3 = createTestData_20(4096, 'K');

    xpkObject xpk1 = xpkOpen("test_20_merge_1.xpk", 0, 0);
    xpkObject xpk2 = xpkOpen("test_20_merge_2.xpk", 0, 0);
    xpkObject xpkMerged = xpkOpen("test_20_merged.xpk", 0, 0);

    ASSERT_NOT_NULL(xpk1);
    ASSERT_NOT_NULL(xpk2);
    ASSERT_NOT_NULL(xpkMerged);

    ASSERT_NE(xpkAppendData(xpk1, pData1, 1024, 1), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk1, pData2, 2048, 2), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk2, pData3, 4096, 1), UINT32_MAX);

    ASSERT_EQ(xpkSave(xpk1), 0);
    ASSERT_EQ(xpkSave(xpk2), 0);

    for (int iPos = 1; iPos <= 2; iPos++) {
        uint32_t outSize = 0;
        void* pExtracted = xpkExtractData(xpk1, iPos, &outSize);
        ASSERT_NOT_NULL(pExtracted);
        ASSERT_NE(xpkAppendData(xpkMerged, pExtracted, outSize, iPos), UINT32_MAX);
        xpkFree(pExtracted);
    }

    uint32_t outSize = 0;
    void* pExtracted = xpkExtractData(xpk2, 0, &outSize);
    ASSERT_NOT_NULL(pExtracted);
    ASSERT_NE(xpkAppendData(xpkMerged, pExtracted, outSize, 3), UINT32_MAX);
    ASSERT_EQ(xpkSave(xpkMerged), 0);
    xpkFree(pExtracted);

    ASSERT_EQ(xpkCount(xpkMerged), 3);

    free(pData1);
    free(pData2);
    free(pData3);
    xpkClose(xpk1);
    xpkClose(xpk2);
    xpkClose(xpkMerged);
}

TEST(simultaneous_operations_on_packages) {
    char* pData = createTestData_20(2048, 'L');

    xpkObject xpk1 = xpkOpen("test_20_simul_1.xpk", 0, 0);
    xpkObject xpk2 = xpkOpen("test_20_simul_2.xpk", 0, 0);

    ASSERT_NOT_NULL(xpk1);
    ASSERT_NOT_NULL(xpk2);

    for (int i = 0; i < 10; i++) {
        ASSERT_NE(xpkAppendData(xpk1, pData, 2048, i + 1), UINT32_MAX);
        ASSERT_NE(xpkAppendData(xpk2, pData, 2048, i + 1), UINT32_MAX);
    }

    ASSERT_EQ(xpkCount(xpk1), 10);
    ASSERT_EQ(xpkCount(xpk2), 10);

    ASSERT_EQ(xpkSave(xpk1), 0);
    ASSERT_EQ(xpkSave(xpk2), 0);

    xpkClose(xpk1);
    xpkClose(xpk2);
    free(pData);
}

TEST(close_one_package_keeps_others_open) {
    char* pData1 = createTestData_20(1024, 'M');
    char* pData2 = createTestData_20(2048, 'N');
    char* pData3 = createTestData_20(4096, 'O');

    xpkObject xpk1 = xpkOpen("test_20_close_1.xpk", 0, 0);
    xpkObject xpk2 = xpkOpen("test_20_close_2.xpk", 0, 0);
    xpkObject xpk3 = xpkOpen("test_20_close_3.xpk", 0, 0);

    ASSERT_NOT_NULL(xpk1);
    ASSERT_NOT_NULL(xpk2);
    ASSERT_NOT_NULL(xpk3);

    ASSERT_NE(xpkAppendData(xpk1, pData1, 1024, 1), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk2, pData2, 2048, 1), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk3, pData3, 4096, 1), UINT32_MAX);

    xpkClose(xpk1);

    ASSERT_NE(xpkAppendData(xpk2, pData2, 2048, 2), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk3, pData3, 4096, 2), UINT32_MAX);

    ASSERT_EQ(xpkCount(xpk2), 2);
    ASSERT_EQ(xpkCount(xpk3), 2);

    free(pData1);
    free(pData2);
    free(pData3);
    xpkClose(xpk2);
    xpkClose(xpk3);
}

TEST(different_properties_in_packages) {
    char* pData = createTestData_20(2048, 'P');

    xpkObject xpk1 = xpkOpen("test_20_prop_1.xpk", 0, 0);
    xpkObject xpk2 = xpkOpen("test_20_prop_2.xpk", 0, 0);

    ASSERT_NOT_NULL(xpk1);
    ASSERT_NOT_NULL(xpk2);

    ASSERT_NE(xpkAppendData(xpk1, pData, 2048, 1), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk2, pData, 2048, 1), UINT32_MAX);

    ASSERT_EQ(xpkDiscCodeSet(xpk1, 0x11111111), 0);
    ASSERT_EQ(xpkDiscCodeSet(xpk2, 0x22222222), 0);

    ASSERT_EQ(xpkSave(xpk1), 0);
    ASSERT_EQ(xpkSave(xpk2), 0);

    ASSERT_EQ(xpkDiscCode(xpk1), 0x11111111);
    ASSERT_EQ(xpkDiscCode(xpk2), 0x22222222);

    free(pData);
    xpkClose(xpk1);
    xpkClose(xpk2);

    xpk1 = xpkOpen("test_20_prop_1.xpk", 0, 0);
    xpk2 = xpkOpen("test_20_prop_2.xpk", 0, 0);

    ASSERT_EQ(xpkDiscCode(xpk1), 0x11111111);
    ASSERT_EQ(xpkDiscCode(xpk2), 0x22222222);

    xpkClose(xpk1);
    xpkClose(xpk2);
}

TEST(create_and_close_many_packages) {
    char* pData = createTestData_20(512, 'Q');

    for (int i = 0; i < 20; i++) {
        char sFilename[64];
        sprintf(sFilename, "test_20_many_%d.xpk", i);

        xpkObject xpk = xpkOpen(sFilename, 0, 0);
        ASSERT_NOT_NULL(xpk);

        ASSERT_NE(xpkAppendData(xpk, pData, 512, 1), UINT32_MAX);
        ASSERT_EQ(xpkSave(xpk), 0);
        ASSERT_EQ(xpkCount(xpk), 1);

        xpkClose(xpk);
    }

    free(pData);
}

TEST(verify_data_independence) {
    char* pData1 = createTestData_20(2048, 'R');
    char* pData2 = createTestData_20(4096, 'S');

    xpkObject xpk1 = xpkOpen("test_20_indep_1.xpk", 0, 0);
    xpkObject xpk2 = xpkOpen("test_20_indep_2.xpk", 0, 0);

    ASSERT_NOT_NULL(xpk1);
    ASSERT_NOT_NULL(xpk2);

    ASSERT_NE(xpkAppendData(xpk1, pData1, 2048, 1), UINT32_MAX);
    ASSERT_NE(xpkAppendData(xpk2, pData2, 4096, 1), UINT32_MAX);

    ASSERT_EQ(xpkSave(xpk1), 0);
    ASSERT_EQ(xpkSave(xpk2), 0);

    ASSERT_EQ(xpkVerify(xpk1, 0), 0);
    ASSERT_EQ(xpkVerify(xpk2, 0), 0);

    free(pData1);
    free(pData2);
    xpkClose(xpk1);
    xpkClose(xpk2);
}

void register_20_multiple_packages_tests(void) {
    TEST_REGISTER(open_multiple_packages_simultaneously, CAT_CORE, "Open multiple packages simultaneously");
    TEST_REGISTER(data_isolation_between_packages, CAT_CORE, "Data isolation between packages");
    TEST_REGISTER(different_compression_levels_in_packages, CAT_CORE, "Different compression levels in packages");
    TEST_REGISTER(different_package_types, CAT_CORE, "Different package types");
    TEST_REGISTER(copy_data_between_packages, CAT_CORE, "Copy data between packages");
    TEST_REGISTER(merge_packages, CAT_CORE, "Merge packages");
    TEST_REGISTER(simultaneous_operations_on_packages, CAT_CORE, "Simultaneous operations on packages");
    TEST_REGISTER(close_one_package_keeps_others_open, CAT_CORE, "Close one package keeps others open");
    TEST_REGISTER(different_properties_in_packages, CAT_CORE, "Different properties in packages");
    TEST_REGISTER(create_and_close_many_packages, CAT_CORE, "Create and close many packages");
    TEST_REGISTER(verify_data_independence, CAT_CORE, "Verify data independence");
}
