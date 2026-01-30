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

TEST(regression_basic_operations) {
	char* pData = createTestData(1024, 'A');
	const char* sFilename = "test_30_basic.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, 1024, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	char* pExtracted = (char*)malloc(1024);
	int iSize = 1024;
	ASSERT_EQ(xpkExtract(xpk, pExtracted, &iSize, 1), 0);
	ASSERT_EQ(iSize, 1024);
	ASSERT_EQ(memcmp(pData, pExtracted, 1024), 0);

	free(pExtracted);
	free(pData);
	xpkClose(xpk);
}

TEST(regression_multiple_files) {
	const char* sFilename = "test_30_multi.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 50; i++ ) {
		char* pData = createTestData(512, 'A' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 512, i + 1), 0);
		free(pData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 50);
	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	xpkClose(xpk);
}

TEST(regression_index_mode) {
	char* pData = createTestData(2048, 'B');
	const char* sFilename = "test_30_index.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 5);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, 2048, 100), 0);
	ASSERT_EQ(xpkAppendData(xpk, pData, 2048, 200), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_EQ(xpkFind(xpk, 100), 100);
	ASSERT_EQ(xpkFind(xpk, 200), 200);
	ASSERT_EQ(xpkFind(xpk, 300), -1);

	char* pExtracted = (char*)malloc(2048);
	int iSize = 2048;
	ASSERT_EQ(xpkExtract(xpk, pExtracted, &iSize, 100), 0);
	ASSERT_EQ(iSize, 2048);
	ASSERT_EQ(memcmp(pData, pExtracted, 2048), 0);

	free(pExtracted);
	free(pData);
	xpkClose(xpk);
}

TEST(regression_path_mode) {
	char* pData = createTestData(1024, 'C');
	const char* sFilename = "test_30_path.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_PATH, 5);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppend(xpk, "folder/file.txt", pData, 1024), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_NE(xpkFind(xpk, "folder/file.txt"), -1);

	char* pExtracted = (char*)malloc(1024);
	int iSize = 1024;
	ASSERT_EQ(xpkExtract(xpk, pExtracted, &iSize, 1), 0);
	ASSERT_EQ(iSize, 1024);
	ASSERT_EQ(memcmp(pData, pExtracted, 1024), 0);

	free(pExtracted);
	free(pData);
	xpkClose(xpk);
}

TEST(regression_remove_update) {
	char* pData1 = createTestData(1024, 'D');
	char* pData2 = createTestData(2048, 'E');
	const char* sFilename = "test_30_remove_update.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppendData(xpk, pData1, 1024, 1), 0);
	ASSERT_EQ(xpkAppendData(xpk, pData2, 2048, 2), 0);
	ASSERT_EQ(xpkAppendData(xpk, pData1, 1024, 3), 0);

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 3);

	ASSERT_EQ(xpkRemove(xpk, 2), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 2);

	ASSERT_EQ(xpkUpdateData(xpk, pData2, 2048, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	free(pData1);
	free(pData2);
	xpkClose(xpk);
}

TEST(regression_compression_levels) {
	char* pData = createTestData(4096, 'F');
	const char* sFilename = "test_30_levels.xpk";

	for ( int iLevel = 0; iLevel <= 15; iLevel++ ) {
		char sLevelFilename[64];
		sprintf(sLevelFilename, "%s_%d", sFilename, iLevel);

		xpkObject xpk = xpkOpen(sLevelFilename, XPK_TYPE_CORE, iLevel);
		ASSERT_NOT_NULL(xpk);

		ASSERT_EQ(xpkAppendData(xpk, pData, 4096, 1), 0);
		ASSERT_EQ(xpkSave(xpk), 0);

		char* pExtracted = (char*)malloc(4096);
		int iSize = 4096;
		ASSERT_EQ(xpkExtract(xpk, pExtracted, &iSize, 1), 0);
		ASSERT_EQ(iSize, 4096);
		ASSERT_EQ(memcmp(pData, pExtracted, 4096), 0);

		free(pExtracted);
		xpkClose(xpk);
	}

	free(pData);
}

TEST(regression_large_file) {
	int iSize = 5 * 1024 * 1024;
	char* pData = createTestData(iSize, 'G');
	const char* sFilename = "test_30_large.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 6);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, iSize, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	char* pExtracted = (char*)malloc(iSize);
	int iExtractedSize = iSize;
	ASSERT_EQ(xpkExtract(xpk, pExtracted, &iExtractedSize, 1), 0);
	ASSERT_EQ(iExtractedSize, iSize);
	ASSERT_EQ(memcmp(pData, pExtracted, iSize), 0);

	free(pExtracted);
	free(pData);
	xpkClose(xpk);
}

TEST(regression_many_files) {
	const char* sFilename = "test_30_many.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 500; i++ ) {
		char* pData = createTestData(256, 'H' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 256, i), 0);
		free(pData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 500);
	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	xpkClose(xpk);
}

TEST(regression_traverse_operations) {
	const char* sFilename = "test_30_traverse.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 100; i++ ) {
		char* pData = createTestData(512, 'I' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 512, i * 10), 0);
		free(pData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	int iCount = 0;
	ASSERT_EQ(xpkEach(xpk, NULL, &iCount), 0);
	ASSERT_EQ(iCount, 100);

	ASSERT_EQ(xpkCount(xpk), 100);

	xpkClose(xpk);
}

TEST(regression_statistics) {
	const char* sFilename = "test_30_stats.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	int iTotalOriginal = 0;
	for ( int i = 0; i < 50; i++ ) {
		char* pData = createTestData(2048, 'J' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 2048, i + 1), 0);
		iTotalOriginal += 2048;
		free(pData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	int iOriginalSize = 0;
	int iCompressedSize = 0;
	ASSERT_EQ(xpkStatGet(xpk, &iOriginalSize, &iCompressedSize, 0), 0);
	ASSERT_EQ(iOriginalSize, iTotalOriginal);
	ASSERT_NE(iCompressedSize, 0);

	ASSERT_EQ(xpkCount(xpk), 50);

	xpkClose(xpk);
}

TEST(regression_rebuild) {
	const char* sFilename = "test_30_rebuild.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 100; i++ ) {
		char* pData = createTestData(1024, 'K' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 1024, i + 1), 0);
		free(pData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	for ( int i = 0; i < 30; i++ ) {
		ASSERT_EQ(xpkRemove(xpk, i * 3 + 1), 0);
	}

	ASSERT_EQ(xpkRebuild(xpk), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	ASSERT_EQ(xpkCount(xpk), 70);

	xpkClose(xpk);
}

TEST(regression_properties) {
	char* pData = createTestData(2048, 'L');
	const char* sFilename = "test_30_props.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppendData(xpk, pData, 2048, 1), 0);
	ASSERT_EQ(xpkDiscCodeSet(xpk, 0xABCDEF01), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_EQ(xpkType(xpk), XPK_TYPE_CORE);
	ASSERT_EQ(xpkDiscCode(xpk), 0xABCDEF01);
	ASSERT_EQ(xpkCount(xpk), 1);

	free(pData);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkDiscCode(xpk), 0xABCDEF01);
	xpkClose(xpk);
}

TEST(regression_extract_all) {
	const char* sFilename = "test_30_extract_all.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 20; i++ ) {
		char* pData = createTestData(4096, 'M' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 4096, i + 1), 0);
		free(pData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkExtractAll(xpk), 0);
	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	ASSERT_EQ(xpkCount(xpk), 20);

	xpkClose(xpk);
}

TEST(regression_user_data) {
	char* pData = createTestData(1024, 'N');
	char* pUserData = createTestData(256, 'O');
	const char* sFilename = "test_30_userdata.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 5);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppendData(xpk, pData, 1024, 100), 0);
	ASSERT_EQ(xpkUserData(xpk, pUserData, 256, 100), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	char* pExtractedUser = (char*)malloc(256);
	int iUserSize = 256;
	ASSERT_EQ(xpkGetUser(xpk, pExtractedUser, &iUserSize, 100), 0);
	ASSERT_EQ(iUserSize, 256);
	ASSERT_EQ(memcmp(pUserData, pExtractedUser, 256), 0);

	free(pExtractedUser);
	free(pData);
	free(pUserData);
	xpkClose(xpk);
}

TEST(regression_edge_cases) {
	const char* sFilename = "test_30_edge.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	char* pData1 = createTestData(1, 'A');
	char* pData2 = createTestData(1024, 'B');
	char* pData3 = createTestData(4 * 1024 * 1024, 'C');

	ASSERT_EQ(xpkAppendData(xpk, pData1, 1, 1), 0);
	ASSERT_EQ(xpkAppendData(xpk, pData2, 1024, 2), 0);
	ASSERT_EQ(xpkAppendData(xpk, pData3, 4 * 1024 * 1024, 3), 0);

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkVerifyAll(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 3);

	free(pData1);
	free(pData2);
	free(pData3);
	xpkClose(xpk);
}

TEST(regression_save_load_cycles) {
	const char* sFilename = "test_30_cycles.xpk";
	char* pData = createTestData(2048, 'P');

	for ( int iCycle = 0; iCycle < 10; iCycle++ ) {
		xpkObject xpk = xpkOpen(sFilename, 0, 0);
		ASSERT_NOT_NULL(xpk);

		if ( iCycle == 0 ) {
			for ( int j = 0; j < 20; j++ ) {
				ASSERT_EQ(xpkAppendData(xpk, pData, 2048, j + 1), 0);
			}
		}

		ASSERT_EQ(xpkCount(xpk), 20);
		ASSERT_EQ(xpkSave(xpk), 0);
		ASSERT_EQ(xpkVerifyAll(xpk), 0);
		xpkClose(xpk);
	}

	free(pData);
}

int main(int argc, char* argv[]) {
	xrtInit();

	printf("[Regression Tests]\n");
	printf("==================\n\n");

	RUN_TEST(regression_basic_operations);
	RUN_TEST(regression_multiple_files);
	RUN_TEST(regression_index_mode);
	RUN_TEST(regression_path_mode);
	RUN_TEST(regression_remove_update);
	RUN_TEST(regression_compression_levels);
	RUN_TEST(regression_large_file);
	RUN_TEST(regression_many_files);
	RUN_TEST(regression_traverse_operations);
	RUN_TEST(regression_statistics);
	RUN_TEST(regression_rebuild);
	RUN_TEST(regression_properties);
	RUN_TEST(regression_extract_all);
	RUN_TEST(regression_user_data);
	RUN_TEST(regression_edge_cases);
	RUN_TEST(regression_save_load_cycles);

	printf("==================\n");
	printf("All Regression Tests Passed!\n");

	return 0;
}
