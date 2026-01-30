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

TEST(single_save_load_cycle) {
	char* pData = createTestData(1024, 'A');
	const char* sFilename = "test_19_single_cycle.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 6);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, 1024, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	char* pExtracted = (char*)malloc(1024);
	int iSize = 1024;
	ASSERT_EQ(xpkExtract(xpk, pExtracted, &iSize, 1), 0);
	ASSERT_EQ(iSize, 1024);
	ASSERT_EQ(memcmp(pData, pExtracted, 1024), 0);

	free(pExtracted);
	free(pData);
	xpkClose(xpk);
}

TEST(multiple_save_load_cycles) {
	char* pData = createTestData(2048, 'B');
	const char* sFilename = "test_19_multiple_cycles.xpk";

	for ( int iCycle = 0; iCycle < 5; iCycle++ ) {
		xpkObject xpk = xpkOpen(sFilename, 0, 0);
		ASSERT_NOT_NULL(xpk);

		char* pExtracted = (char*)malloc(2048);
		int iSize = 2048;

		if ( iCycle > 0 ) {
			ASSERT_EQ(xpkExtract(xpk, pExtracted, &iSize, 1), 0);
			ASSERT_EQ(iSize, 2048);
			ASSERT_EQ(memcmp(pData, pExtracted, 2048), 0);
			free(pExtracted);
		}

		ASSERT_EQ(xpkSave(xpk), 0);
		xpkClose(xpk);
	}

	free(pData);
}

TEST(save_load_with_modifications) {
	char* pData1 = createTestData(1024, 'C');
	char* pData2 = createTestData(2048, 'D');
	const char* sFilename = "test_19_with_modifications.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData1, 1024, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData2, 2048, 2), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	char* pExtracted1 = (char*)malloc(1024);
	char* pExtracted2 = (char*)malloc(2048);
	int iSize1 = 1024;
	int iSize2 = 2048;

	ASSERT_EQ(xpkExtract(xpk, pExtracted1, &iSize1, 1), 0);
	ASSERT_EQ(xpkExtract(xpk, pExtracted2, &iSize2, 2), 0);

	ASSERT_EQ(iSize1, 1024);
	ASSERT_EQ(iSize2, 2048);
	ASSERT_EQ(memcmp(pData1, pExtracted1, 1024), 0);
	ASSERT_EQ(memcmp(pData2, pExtracted2, 2048), 0);

	free(pExtracted1);
	free(pExtracted2);
	free(pData1);
	free(pData2);
	xpkClose(xpk);
}

TEST(save_load_after_remove) {
	char* pData1 = createTestData(512, 'E');
	char* pData2 = createTestData(1024, 'F');
	char* pData3 = createTestData(2048, 'G');
	const char* sFilename = "test_19_after_remove.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 4);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData1, 512, 1), 0);
	ASSERT_EQ(xpkAppendData(xpk, pData2, 1024, 2), 0);
	ASSERT_EQ(xpkAppendData(xpk, pData3, 2048, 3), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkRemove(xpk, 2), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	char* pExtracted1 = (char*)malloc(512);
	char* pExtracted3 = (char*)malloc(2048);
	int iSize1 = 512;
	int iSize3 = 2048;

	ASSERT_EQ(xpkExtract(xpk, pExtracted1, &iSize1, 1), 0);
	ASSERT_EQ(xpkExtract(xpk, pExtracted3, &iSize3, 3), 0);

	ASSERT_EQ(iSize1, 512);
	ASSERT_EQ(iSize3, 2048);
	ASSERT_EQ(memcmp(pData1, pExtracted1, 512), 0);
	ASSERT_EQ(memcmp(pData3, pExtracted3, 2048), 0);

	free(pExtracted1);
	free(pExtracted3);
	free(pData1);
	free(pData2);
	free(pData3);
	xpkClose(xpk);
}

TEST(save_load_after_update) {
	char* pData = createTestData(1024, 'H');
	char* pNewData = createTestData(2048, 'I');
	const char* sFilename = "test_19_after_update.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, 1024, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkUpdateData(xpk, pNewData, 2048, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	char* pExtracted = (char*)malloc(2048);
	int iSize = 2048;

	ASSERT_EQ(xpkExtract(xpk, pExtracted, &iSize, 1), 0);
	ASSERT_EQ(iSize, 2048);
	ASSERT_EQ(memcmp(pNewData, pExtracted, 2048), 0);

	free(pExtracted);
	free(pData);
	free(pNewData);
	xpkClose(xpk);
}

TEST(save_load_different_compression_levels) {
	char* pData = createTestData(8192, 'J');
	const char* sFilename = "test_19_compression_levels.xpk";

	for ( int iLevel = 0; iLevel <= 15; iLevel++ ) {
		xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, iLevel);
		ASSERT_NOT_NULL(xpk);
		ASSERT_EQ(xpkAppendData(xpk, pData, 8192, 1), 0);
		ASSERT_EQ(xpkSave(xpk), 0);
		xpkClose(xpk);

		xpk = xpkOpen(sFilename, 0, 0);
		ASSERT_NOT_NULL(xpk);

		char* pExtracted = (char*)malloc(8192);
		int iSize = 8192;

		ASSERT_EQ(xpkExtract(xpk, pExtracted, &iSize, 1), 0);
		ASSERT_EQ(iSize, 8192);
		ASSERT_EQ(memcmp(pData, pExtracted, 8192), 0);

		free(pExtracted);
		xpkClose(xpk);
	}

	free(pData);
}

TEST(save_load_index_mode) {
	char* pData1 = createTestData(1024, 'K');
	char* pData2 = createTestData(2048, 'L');
	const char* sFilename = "test_19_index_mode.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 5);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData1, 1024, 100), 0);
	ASSERT_EQ(xpkAppendData(xpk, pData2, 2048, 200), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	char* pExtracted1 = (char*)malloc(1024);
	char* pExtracted2 = (char*)malloc(2048);
	int iSize1 = 1024;
	int iSize2 = 2048;

	ASSERT_EQ(xpkExtract(xpk, pExtracted1, &iSize1, 100), 0);
	ASSERT_EQ(xpkExtract(xpk, pExtracted2, &iSize2, 200), 0);

	ASSERT_EQ(iSize1, 1024);
	ASSERT_EQ(iSize2, 2048);
	ASSERT_EQ(memcmp(pData1, pExtracted1, 1024), 0);
	ASSERT_EQ(memcmp(pData2, pExtracted2, 2048), 0);

	free(pExtracted1);
	free(pExtracted2);
	free(pData1);
	free(pData2);
	xpkClose(xpk);
}

TEST(save_load_path_mode) {
	char* pData1 = createTestData(1024, 'M');
	char* pData2 = createTestData(2048, 'N');
	const char* sFilename = "test_19_path_mode.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_PATH, 5);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData1, 1024, 1), 0);
	ASSERT_EQ(xpkAppendData(xpk, pData2, 2048, 2), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	char* pExtracted1 = (char*)malloc(1024);
	char* pExtracted2 = (char*)malloc(2048);
	int iSize1 = 1024;
	int iSize2 = 2048;

	ASSERT_EQ(xpkExtract(xpk, pExtracted1, &iSize1, 1), 0);
	ASSERT_EQ(xpkExtract(xpk, pExtracted2, &iSize2, 2), 0);

	ASSERT_EQ(iSize1, 1024);
	ASSERT_EQ(iSize2, 2048);
	ASSERT_EQ(memcmp(pData1, pExtracted1, 1024), 0);
	ASSERT_EQ(memcmp(pData2, pExtracted2, 2048), 0);

	free(pExtracted1);
	free(pExtracted2);
	free(pData1);
	free(pData2);
	xpkClose(xpk);
}

TEST(save_load_with_properties) {
	char* pData = createTestData(4096, 'O');
	const char* sFilename = "test_19_properties.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 6);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, 4096, 1), 0);
	ASSERT_EQ(xpkDiscCodeSet(xpk, 0x12345678), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkDiscCode(xpk), 0x12345678);

	char* pExtracted = (char*)malloc(4096);
	int iSize = 4096;

	ASSERT_EQ(xpkExtract(xpk, pExtracted, &iSize, 1), 0);
	ASSERT_EQ(iSize, 4096);
	ASSERT_EQ(memcmp(pData, pExtracted, 4096), 0);

	free(pExtracted);
	free(pData);
	xpkClose(xpk);
}

TEST(save_load_empty_package) {
	const char* sFilename = "test_19_empty.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 0);
	xpkClose(xpk);
}

TEST(save_load_large_file) {
	char* pData = createTestData(5 * 1024 * 1024, 'P');
	const char* sFilename = "test_19_large_file.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 6);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, 5 * 1024 * 1024, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	char* pExtracted = (char*)malloc(5 * 1024 * 1024);
	int iSize = 5 * 1024 * 1024;

	ASSERT_EQ(xpkExtract(xpk, pExtracted, &iSize, 1), 0);
	ASSERT_EQ(iSize, 5 * 1024 * 1024);
	ASSERT_EQ(memcmp(pData, pExtracted, 5 * 1024 * 1024), 0);

	free(pExtracted);
	free(pData);
	xpkClose(xpk);
}

TEST(save_load_user_data) {
	char* pData = createTestData(2048, 'Q');
	char* pUserData = createTestData(128, 'R');
	const char* sFilename = "test_19_user_data.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 5);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, 2048, 10), 0);
	ASSERT_EQ(xpkUserData(xpk, pUserData, 128, 10), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	char* pExtractedData = (char*)malloc(2048);
	char* pExtractedUserData = (char*)malloc(128);
	int iDataSize = 2048;
	int iUserSize = 128;

	ASSERT_EQ(xpkExtract(xpk, pExtractedData, &iDataSize, 10), 0);
	ASSERT_EQ(xpkGetUser(xpk, pExtractedUserData, &iUserSize, 10), 0);

	ASSERT_EQ(iDataSize, 2048);
	ASSERT_EQ(iUserSize, 128);
	ASSERT_EQ(memcmp(pData, pExtractedData, 2048), 0);
	ASSERT_EQ(memcmp(pUserData, pExtractedUserData, 128), 0);

	free(pExtractedData);
	free(pExtractedUserData);
	free(pData);
	free(pUserData);
	xpkClose(xpk);
}

int main(int argc, char* argv[]) {
	xrtInit();

	printf("[Save/Load Cycles Tests]\n");
	printf("=========================\n\n");

	RUN_TEST(single_save_load_cycle);
	RUN_TEST(multiple_save_load_cycles);
	RUN_TEST(save_load_with_modifications);
	RUN_TEST(save_load_after_remove);
	RUN_TEST(save_load_after_update);
	RUN_TEST(save_load_different_compression_levels);
	RUN_TEST(save_load_index_mode);
	RUN_TEST(save_load_path_mode);
	RUN_TEST(save_load_with_properties);
	RUN_TEST(save_load_empty_package);
	RUN_TEST(save_load_large_file);
	RUN_TEST(save_load_user_data);

	printf("=========================\n");
	printf("All Save/Load Cycles Tests Passed!\n");

	return 0;
}
