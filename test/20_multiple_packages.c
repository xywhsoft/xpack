#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xpack.h"
#include "xrt.h"

#define TEST(name) void name()
#define RUN_TEST(name) do { printf("Running: %s\n", #name); name(); printf("PASSED\n\n"); } while(0)
#define ASSERT_EQ(a, b) do { if ((a) != (b)) { printf("FAIL at %s:%d - Expected %d, got %d\n", __FILE__, __LINE__, (int)(b), (int)(a)); return; } } while(0)
#define ASSERT_NE(a, b) do { if ((a) == (b)) { printf("FAIL at %s:%d - Not expected %d\n", __FILE__, __LINE__, (int)(b)); return; } } while(0)
#define ASSERT_NOT_NULL(a) do { if ((a) == NULL) { printf("FAIL at %s:%d - NULL value\n", __FILE__, __LINE__); return; } } while(0)

static char* createTestData(int size, char pattern) {
	char* pData = (char*)malloc(size);
	for ( int i = 0; i < size; i++ ) {
		pData[i] = (char)(pattern + (i % 26));
	}
	return pData;
}

TEST(open_multiple_packages_simultaneously) {
	char* pData1 = createTestData(1024, 'A');
	char* pData2 = createTestData(2048, 'B');
	char* pData3 = createTestData(4096, 'C');

	xpkObject xpk1 = xpkOpen("test_20_multi_1.xpk", XPK_TYPE_CORE, 5);
	xpkObject xpk2 = xpkOpen("test_20_multi_2.xpk", XPK_TYPE_INDEX, 6);
	xpkObject xpk3 = xpkOpen("test_20_multi_3.xpk", XPK_TYPE_PATH, 7);

	ASSERT_NOT_NULL(xpk1);
	ASSERT_NOT_NULL(xpk2);
	ASSERT_NOT_NULL(xpk3);

	ASSERT_EQ(xpkAppendData(xpk1, pData1, 1024, 1), 0);
	ASSERT_EQ(xpkAppendData(xpk2, pData2, 2048, 10), 0);
	ASSERT_EQ(xpkAppendData(xpk3, pData3, 4096, 1), 0);

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
	char* pData1 = createTestData(1024, 'D');
	char* pData2 = createTestData(2048, 'E');

	xpkObject xpk1 = xpkOpen("test_20_isolation_1.xpk", XPK_TYPE_CORE, 5);
	xpkObject xpk2 = xpkOpen("test_20_isolation_2.xpk", XPK_TYPE_CORE, 5);

	ASSERT_NOT_NULL(xpk1);
	ASSERT_NOT_NULL(xpk2);

	ASSERT_EQ(xpkAppendData(xpk1, pData1, 1024, 1), 0);
	ASSERT_EQ(xpkAppendData(xpk2, pData2, 2048, 1), 0);

	ASSERT_EQ(xpkSave(xpk1), 0);
	ASSERT_EQ(xpkSave(xpk2), 0);

	ASSERT_EQ(xpkCount(xpk1), 1);
	ASSERT_EQ(xpkCount(xpk2), 1);

	char* pExtracted1 = (char*)malloc(1024);
	char* pExtracted2 = (char*)malloc(2048);
	int iSize1 = 1024;
	int iSize2 = 2048;

	ASSERT_EQ(xpkExtract(xpk1, pExtracted1, &iSize1, 1), 0);
	ASSERT_EQ(xpkExtract(xpk2, pExtracted2, &iSize2, 1), 0);

	ASSERT_EQ(memcmp(pData1, pExtracted1, 1024), 0);
	ASSERT_EQ(memcmp(pData2, pExtracted2, 2048), 0);

	free(pExtracted1);
	free(pExtracted2);
	free(pData1);
	free(pData2);
	xpkClose(xpk1);
	xpkClose(xpk2);
}

TEST_different_compression_levels_in_packages) {
	char* pData = createTestData(4096, 'F');

	xpkObject xpk1 = xpkOpen("test_20_comp_1.xpk", XPK_TYPE_CORE, 0);
	xpkObject xpk2 = xpkOpen("test_20_comp_2.xpk", XPK_TYPE_CORE, 5);
	xpkObject xpk3 = xpkOpen("test_20_comp_3.xpk", XPK_TYPE_CORE, 15);

	ASSERT_NOT_NULL(xpk1);
	ASSERT_NOT_NULL(xpk2);
	ASSERT_NOT_NULL(xpk3);

	ASSERT_EQ(xpkAppendData(xpk1, pData, 4096, 1), 0);
	ASSERT_EQ(xpkAppendData(xpk2, pData, 4096, 1), 0);
	ASSERT_EQ(xpkAppendData(xpk3, pData, 4096, 1), 0);

	ASSERT_EQ(xpkSave(xpk1), 0);
	ASSERT_EQ(xpkSave(xpk2), 0);
	ASSERT_EQ(xpkSave(xpk3), 0);

	xpkClose(xpk1);
	xpkClose(xpk2);
	xpkClose(xpk3);

	char* pExtracted1 = (char*)malloc(4096);
	char* pExtracted2 = (char*)malloc(4096);
	char* pExtracted3 = (char*)malloc(4096);
	int iSize1 = 4096;
	int iSize2 = 4096;
	int iSize3 = 4096;

	xpk1 = xpkOpen("test_20_comp_1.xpk", 0, 0);
	xpk2 = xpkOpen("test_20_comp_2.xpk", 0, 0);
	xpk3 = xpkOpen("test_20_comp_3.xpk", 0, 0);

	ASSERT_NOT_NULL(xpk1);
	ASSERT_NOT_NULL(xpk2);
	ASSERT_NOT_NULL(xpk3);

	ASSERT_EQ(xpkExtract(xpk1, pExtracted1, &iSize1, 1), 0);
	ASSERT_EQ(xpkExtract(xpk2, pExtracted2, &iSize2, 1), 0);
	ASSERT_EQ(xpkExtract(xpk3, pExtracted3, &iSize3, 1), 0);

	ASSERT_EQ(iSize1, 4096);
	ASSERT_EQ(iSize2, 4096);
	ASSERT_EQ(iSize3, 4096);
	ASSERT_EQ(memcmp(pData, pExtracted1, 4096), 0);
	ASSERT_EQ(memcmp(pData, pExtracted2, 4096), 0);
	ASSERT_EQ(memcmp(pData, pExtracted3, 4096), 0);

	free(pExtracted1);
	free(pExtracted2);
	free(pExtracted3);
	free(pData);
	xpkClose(xpk1);
	xpkClose(xpk2);
	xpkClose(xpk3);
}

TEST(different_package_types) {
	char* pData = createTestData(2048, 'G');

	xpkObject xpkCore = xpkOpen("test_20_type_core.xpk", XPK_TYPE_CORE, 5);
	xpkObject xpkIndex = xpkOpen("test_20_type_index.xpk", XPK_TYPE_INDEX, 5);
	xpkObject xpkPath = xpkOpen("test_20_type_path.xpk", XPK_TYPE_PATH, 5);

	ASSERT_NOT_NULL(xpkCore);
	ASSERT_NOT_NULL(xpkIndex);
	ASSERT_NOT_NULL(xpkPath);

	ASSERT_EQ(xpkAppendData(xpkCore, pData, 2048, 1), 0);
	ASSERT_EQ(xpkAppendData(xpkIndex, pData, 2048, 100), 0);
	ASSERT_EQ(xpkAppendData(xpkPath, pData, 2048, 1), 0);

	ASSERT_EQ(xpkSave(xpkCore), 0);
	ASSERT_EQ(xpkSave(xpkIndex), 0);
	ASSERT_EQ(xpkSave(xpkPath), 0);

	ASSERT_EQ(xpkType(xpkCore), XPK_TYPE_CORE);
	ASSERT_EQ(xpkType(xpkIndex), XPK_TYPE_INDEX);
	ASSERT_EQ(xpkType(xpkPath), XPK_TYPE_PATH);

	xpkClose(xpkCore);
	xpkClose(xpkIndex);
	xpkClose(xpkPath);

	char* pExtractedCore = (char*)malloc(2048);
	char* pExtractedIndex = (char*)malloc(2048);
	char* pExtractedPath = (char*)malloc(2048);
	int iSizeCore = 2048;
	int iSizeIndex = 2048;
	int iSizePath = 2048;

	xpkCore = xpkOpen("test_20_type_core.xpk", 0, 0);
	xpkIndex = xpkOpen("test_20_type_index.xpk", 0, 0);
	xpkPath = xpkOpen("test_20_type_path.xpk", 0, 0);

	ASSERT_NOT_NULL(xpkCore);
	ASSERT_NOT_NULL(xpkIndex);
	ASSERT_NOT_NULL(xpkPath);

	ASSERT_EQ(xpkExtract(xpkCore, pExtractedCore, &iSizeCore, 1), 0);
	ASSERT_EQ(xpkExtract(xpkIndex, pExtractedIndex, &iSizeIndex, 100), 0);
	ASSERT_EQ(xpkExtract(xpkPath, pExtractedPath, &iSizePath, 1), 0);

	ASSERT_EQ(iSizeCore, 2048);
	ASSERT_EQ(iSizeIndex, 2048);
	ASSERT_EQ(iSizePath, 2048);
	ASSERT_EQ(memcmp(pData, pExtractedCore, 2048), 0);
	ASSERT_EQ(memcmp(pData, pExtractedIndex, 2048), 0);
	ASSERT_EQ(memcmp(pData, pExtractedPath, 2048), 0);

	free(pExtractedCore);
	free(pExtractedIndex);
	free(pExtractedPath);
	free(pData);
	xpkClose(xpkCore);
	xpkClose(xpkIndex);
	xpkClose(xpkPath);
}

TEST(copy_data_between_packages) {
	char* pData = createTestData(3072, 'H');

	xpkObject xpkSrc = xpkOpen("test_20_src.xpk", XPK_TYPE_CORE, 5);
	xpkObject xpkDst = xpkOpen("test_20_dst.xpk", XPK_TYPE_CORE, 5);

	ASSERT_NOT_NULL(xpkSrc);
	ASSERT_NOT_NULL(xpkDst);

	ASSERT_EQ(xpkAppendData(xpkSrc, pData, 3072, 1), 0);
	ASSERT_EQ(xpkSave(xpkSrc), 0);

	char* pExtracted = (char*)malloc(3072);
	int iSize = 3072;
	ASSERT_EQ(xpkExtract(xpkSrc, pExtracted, &iSize, 1), 0);
	ASSERT_EQ(xpkAppendData(xpkDst, pExtracted, iSize, 1), 0);
	ASSERT_EQ(xpkSave(xpkDst), 0);

	ASSERT_EQ(xpkCount(xpkSrc), 1);
	ASSERT_EQ(xpkCount(xpkDst), 1);

	free(pExtracted);
	free(pData);
	xpkClose(xpkSrc);
	xpkClose(xpkDst);
}

TEST(merge_packages) {
	char* pData1 = createTestData(1024, 'I');
	char* pData2 = createTestData(2048, 'J');
	char* pData3 = createTestData(4096, 'K');

	xpkObject xpk1 = xpkOpen("test_20_merge_1.xpk", XPK_TYPE_CORE, 5);
	xpkObject xpk2 = xpkOpen("test_20_merge_2.xpk", XPK_TYPE_CORE, 5);
	xpkObject xpkMerged = xpkOpen("test_20_merged.xpk", XPK_TYPE_CORE, 5);

	ASSERT_NOT_NULL(xpk1);
	ASSERT_NOT_NULL(xpk2);
	ASSERT_NOT_NULL(xpkMerged);

	ASSERT_EQ(xpkAppendData(xpk1, pData1, 1024, 1), 0);
	ASSERT_EQ(xpkAppendData(xpk1, pData2, 2048, 2), 0);
	ASSERT_EQ(xpkAppendData(xpk2, pData3, 4096, 1), 0);

	ASSERT_EQ(xpkSave(xpk1), 0);
	ASSERT_EQ(xpkSave(xpk2), 0);

	for ( int iPos = 1; iPos <= 2; iPos++ ) {
		char* pExtracted = (char*)malloc(8192);
		int iSize = 8192;
		ASSERT_EQ(xpkExtract(xpk1, pExtracted, &iSize, iPos), 0);
		ASSERT_EQ(xpkAppendData(xpkMerged, pExtracted, iSize, iPos), 0);
		free(pExtracted);
	}

	char* pExtracted = (char*)malloc(8192);
	int iSize = 8192;
	ASSERT_EQ(xpkExtract(xpk2, pExtracted, &iSize, 1), 0);
	ASSERT_EQ(xpkAppendData(xpkMerged, pExtracted, iSize, 3), 0);
	ASSERT_EQ(xpkSave(xpkMerged), 0);

	ASSERT_EQ(xpkCount(xpkMerged), 3);

	free(pExtracted);
	free(pData1);
	free(pData2);
	free(pData3);
	xpkClose(xpk1);
	xpkClose(xpk2);
	xpkClose(xpkMerged);
}

TEST(simultaneous_operations_on_packages) {
	char* pData = createTestData(2048, 'L');

	xpkObject xpk1 = xpkOpen("test_20_simul_1.xpk", XPK_TYPE_CORE, 5);
	xpkObject xpk2 = xpkOpen("test_20_simul_2.xpk", XPK_TYPE_CORE, 5);

	ASSERT_NOT_NULL(xpk1);
	ASSERT_NOT_NULL(xpk2);

	for ( int i = 0; i < 10; i++ ) {
		ASSERT_EQ(xpkAppendData(xpk1, pData, 2048, i + 1), 0);
		ASSERT_EQ(xpkAppendData(xpk2, pData, 2048, i + 1), 0);
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
	char* pData1 = createTestData(1024, 'M');
	char* pData2 = createTestData(2048, 'N');
	char* pData3 = createTestData(4096, 'O');

	xpkObject xpk1 = xpkOpen("test_20_close_1.xpk", XPK_TYPE_CORE, 5);
	xpkObject xpk2 = xpkOpen("test_20_close_2.xpk", XPK_TYPE_CORE, 5);
	xpkObject xpk3 = xpkOpen("test_20_close_3.xpk", XPK_TYPE_CORE, 5);

	ASSERT_NOT_NULL(xpk1);
	ASSERT_NOT_NULL(xpk2);
	ASSERT_NOT_NULL(xpk3);

	ASSERT_EQ(xpkAppendData(xpk1, pData1, 1024, 1), 0);
	ASSERT_EQ(xpkAppendData(xpk2, pData2, 2048, 1), 0);
	ASSERT_EQ(xpkAppendData(xpk3, pData3, 4096, 1), 0);

	xpkClose(xpk1);

	ASSERT_EQ(xpkAppendData(xpk2, pData2, 2048, 2), 0);
	ASSERT_EQ(xpkAppendData(xpk3, pData3, 4096, 2), 0);

	ASSERT_EQ(xpkCount(xpk2), 2);
	ASSERT_EQ(xpkCount(xpk3), 2);

	free(pData1);
	free(pData2);
	free(pData3);
	xpkClose(xpk2);
	xpkClose(xpk3);
}

TEST(different_properties_in_packages) {
	char* pData = createTestData(2048, 'P');

	xpkObject xpk1 = xpkOpen("test_20_prop_1.xpk", XPK_TYPE_CORE, 5);
	xpkObject xpk2 = xpkOpen("test_20_prop_2.xpk", XPK_TYPE_CORE, 5);

	ASSERT_NOT_NULL(xpk1);
	ASSERT_NOT_NULL(xpk2);

	ASSERT_EQ(xpkAppendData(xpk1, pData, 2048, 1), 0);
	ASSERT_EQ(xpkAppendData(xpk2, pData, 2048, 1), 0);

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
	char* pData = createTestData(512, 'Q');

	for ( int i = 0; i < 20; i++ ) {
		char sFilename[64];
		sprintf(sFilename, "test_20_many_%d.xpk", i);

		xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
		ASSERT_NOT_NULL(xpk);

		ASSERT_EQ(xpkAppendData(xpk, pData, 512, 1), 0);
		ASSERT_EQ(xpkSave(xpk), 0);
		ASSERT_EQ(xpkCount(xpk), 1);

		xpkClose(xpk);
	}

	free(pData);
}

TEST(verify_data_independence) {
	char* pData1 = createTestData(2048, 'R');
	char* pData2 = createTestData(4096, 'S');

	xpkObject xpk1 = xpkOpen("test_20_indep_1.xpk", XPK_TYPE_CORE, 5);
	xpkObject xpk2 = xpkOpen("test_20_indep_2.xpk", XPK_TYPE_CORE, 5);

	ASSERT_NOT_NULL(xpk1);
	ASSERT_NOT_NULL(xpk2);

	ASSERT_EQ(xpkAppendData(xpk1, pData1, 2048, 1), 0);
	ASSERT_EQ(xpkAppendData(xpk2, pData2, 4096, 1), 0);

	ASSERT_EQ(xpkSave(xpk1), 0);
	ASSERT_EQ(xpkSave(xpk2), 0);

	ASSERT_EQ(xpkVerify(xpk1, 1), 0);
	ASSERT_EQ(xpkVerify(xpk2, 1), 0);

	free(pData1);
	free(pData2);
	xpkClose(xpk1);
	xpkClose(xpk2);
}

int main(int argc, char* argv[]) {
	xrtInit();

	printf("[Multiple Packages Tests]\n");
	printf("=========================\n\n");

	RUN_TEST(open_multiple_packages_simultaneously);
	RUN_TEST(data_isolation_between_packages);
	RUN_TEST(different_compression_levels_in_packages);
	RUN_TEST(different_package_types);
	RUN_TEST(copy_data_between_packages);
	RUN_TEST(merge_packages);
	RUN_TEST(simultaneous_operations_on_packages);
	RUN_TEST(close_one_package_keeps_others_open);
	RUN_TEST(different_properties_in_packages);
	RUN_TEST(create_and_close_many_packages);
	RUN_TEST(verify_data_independence);

	printf("=========================\n");
	printf("All Multiple Packages Tests Passed!\n");

	return 0;
}
