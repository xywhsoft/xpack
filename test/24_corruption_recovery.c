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

TEST(create_valid_package_for_corruption_test) {
	char* pData = createTestData(2048, 'A');
	const char* sFilename = "test_24_valid.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, 2048, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	free(pData);
	xpkClose(xpk);
}

TEST(corrupted_signature_detection) {
	const char* sFilename = "test_24_corrupt_sig.xpk";

	FILE* pFile = fopen(sFilename, "wb");
	if ( pFile ) {
		fwrite("BAD", 1, 3, pFile);
		fclose(pFile);
	}

	xpkObject xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_EQ(xpk, NULL);
}

TEST(truncated_file_detection) {
	char* pData = createTestData(4096, 'B');
	const char* sSrcFilename = "test_24_src.xpk";
	const char* sTruncFilename = "test_24_truncated.xpk";

	xpkObject xpk = xpkOpen(sSrcFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, 4096, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	FILE* pSrc = fopen(sSrcFilename, "rb");
	FILE* pDst = fopen(sTruncFilename, "wb");
	if ( pSrc && pDst ) {
		char pBuffer[1024];
		int iRead = fread(pBuffer, 1, 512, pSrc);
		fwrite(pBuffer, 1, iRead, pDst);
	}
	if ( pSrc ) fclose(pSrc);
	if ( pDst ) fclose(pDst);

	xpk = xpkOpen(sTruncFilename, 0, 0);
	ASSERT_EQ(xpk, NULL);

	free(pData);
}

TEST(damaged_data_recovery_with_verify) {
	char* pData = createTestData(2048, 'C');
	const char* sFilename = "test_24_damaged.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, 2048, 1), 0);
	ASSERT_EQ(xpkAppendData(xpk, pData, 2048, 2), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_EQ(xpkVerify(xpk, 1), 0);
	ASSERT_EQ(xpkVerify(xpk, 2), 0);

	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	char* pExtracted = (char*)malloc(2048);
	int iSize = 2048;
	ASSERT_EQ(xpkExtract(xpk, pExtracted, &iSize, 1), 0);
	ASSERT_EQ(iSize, 2048);
	ASSERT_EQ(memcmp(pData, pExtracted, 2048), 0);

	free(pExtracted);
	free(pData);
	xpkClose(xpk);
}

TEST(remove_corrupted_file) {
	char* pData = createTestData(1024, 'D');
	const char* sFilename = "test_24_remove_corrupt.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, 1024, 1), 0);
	ASSERT_EQ(xpkAppendData(xpk, pData, 1024, 2), 0);
	ASSERT_EQ(xpkAppendData(xpk, pData, 1024, 3), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_EQ(xpkRemove(xpk, 2), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	ASSERT_EQ(xpkCount(xpk), 2);

	free(pData);
	xpkClose(xpk);
}

TEST(rebuild_after_partial_damage) {
	char* pData = createTestData(2048, 'E');
	const char* sFilename = "test_24_rebuild.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 10; i++ ) {
		ASSERT_EQ(xpkAppendData(xpk, pData, 2048, i + 1), 0);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_EQ(xpkRemove(xpk, 3), 0);
	ASSERT_EQ(xpkRemove(xpk, 7), 0);

	ASSERT_EQ(xpkRebuild(xpk), 0);
	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	ASSERT_EQ(xpkCount(xpk), 8);

	free(pData);
	xpkClose(xpk);
}

TEST(extract_all_after_removal) {
	char* pData = createTestData(1536, 'F');
	const char* sFilename = "test_24_extract_all.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 5; i++ ) {
		ASSERT_EQ(xpkAppendData(xpk, pData, 1536, i + 1), 0);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_EQ(xpkRemove(xpk, 2), 0);
	ASSERT_EQ(xpkRemove(xpk, 4), 0);

	ASSERT_EQ(xpkVerifyAll(xpk), 0);
	ASSERT_EQ(xpkExtractAll(xpk), 0);

	ASSERT_EQ(xpkCount(xpk), 3);

	free(pData);
	xpkClose(xpk);
}

TEST(save_and_reload_after_damage) {
	char* pData1 = createTestData(1024, 'G');
	char* pData2 = createTestData(2048, 'H');
	const char* sFilename = "test_24_save_reload.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData1, 1024, 1), 0);
	ASSERT_EQ(xpkAppendData(xpk, pData2, 2048, 2), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_EQ(xpkRemove(xpk, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 1);
	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	char* pExtracted = (char*)malloc(2048);
	int iSize = 2048;
	ASSERT_EQ(xpkExtract(xpk, pExtracted, &iSize, 2), 0);
	ASSERT_EQ(iSize, 2048);
	ASSERT_EQ(memcmp(pData2, pExtracted, 2048), 0);

	free(pExtracted);
	free(pData1);
	free(pData2);
	xpkClose(xpk);
}

TEST(rebuild_optimizes_structure) {
	char* pData = createTestData(1024, 'I');
	const char* sFilename = "test_24_rebuild_opt.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 20; i++ ) {
		ASSERT_EQ(xpkAppendData(xpk, pData, 1024, i + 1), 0);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	for ( int i = 0; i < 10; i++ ) {
		ASSERT_EQ(xpkRemove(xpk, i * 2 + 1), 0);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_EQ(xpkRebuild(xpk), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	ASSERT_EQ(xpkCount(xpk), 10);

	free(pData);
	xpkClose(xpk);
}

TEST(statistics_after_recovery) {
	char* pData = createTestData(2048, 'J');
	const char* sFilename = "test_24_stats_recover.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 10; i++ ) {
		ASSERT_EQ(xpkAppendData(xpk, pData, 2048, i + 1), 0);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_EQ(xpkRemove(xpk, 5), 0);
	ASSERT_EQ(xpkRemove(xpk, 10), 0);

	ASSERT_EQ(xpkRebuild(xpk), 0);

	int iOriginalSize = 0;
	int iCompressedSize = 0;
	ASSERT_EQ(xpkStatGet(xpk, &iOriginalSize, &iCompressedSize, 0), 0);
	ASSERT_EQ(iOriginalSize, 8 * 2048);
	ASSERT_NE(iCompressedSize, 0);

	ASSERT_EQ(xpkCount(xpk), 8);

	free(pData);
	xpkClose(xpk);
}

TEST(traverse_after_recovery) {
	char* pData = createTestData(1024, 'K');
	const char* sFilename = "test_24_traverse_recover.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 15; i++ ) {
		ASSERT_EQ(xpkAppendData(xpk, pData, 1024, i * 10), 0);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_EQ(xpkRemove(xpk, 50), 0);
	ASSERT_EQ(xpkRemove(xpk, 100), 0);

	ASSERT_EQ(xpkRebuild(xpk), 0);

	int iCount = 0;
	ASSERT_EQ(xpkEach(xpk, NULL, &iCount), 0);
	ASSERT_EQ(iCount, 13);

	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	free(pData);
	xpkClose(xpk);
}

TEST(update_after_recovery) {
	char* pData1 = createTestData(1024, 'L');
	char* pData2 = createTestData(2048, 'M');
	const char* sFilename = "test_24_update_recover.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 5; i++ ) {
		ASSERT_EQ(xpkAppendData(xpk, pData1, 1024, i + 1), 0);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_EQ(xpkRemove(xpk, 3), 0);

	ASSERT_EQ(xpkUpdateData(xpk, pData2, 2048, 1), 0);
	ASSERT_EQ(xpkUpdateData(xpk, pData2, 2048, 5), 0);

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	ASSERT_EQ(xpkCount(xpk), 4);

	free(pData1);
	free(pData2);
	xpkClose(xpk);
}

TEST(find_after_recovery) {
	char* pData = createTestData(1024, 'N');
	const char* sFilename = "test_24_find_recover.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 5);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppendData(xpk, pData, 1024, 100), 0);
	ASSERT_EQ(xpkAppendData(xpk, pData, 1024, 200), 0);
	ASSERT_EQ(xpkAppendData(xpk, pData, 1024, 300), 0);

	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_EQ(xpkRemove(xpk, 200), 0);
	ASSERT_EQ(xpkRebuild(xpk), 0);

	ASSERT_EQ(xpkFind(xpk, 100), 100);
	ASSERT_EQ(xpkFind(xpk, 200), -1);
	ASSERT_EQ(xpkFind(xpk, 300), 300);

	ASSERT_EQ(xpkCount(xpk), 2);

	free(pData);
	xpkClose(xpk);
}

TEST(multiple_recovery_cycles) {
	char* pData = createTestData(1536, 'O');
	const char* sFilename = "test_24_multi_recover.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 20; i++ ) {
		ASSERT_EQ(xpkAppendData(xpk, pData, 1536, i + 1), 0);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	for ( int iCycle = 0; iCycle < 3; iCycle++ ) {
		for ( int i = 0; i < 5; i++ ) {
			ASSERT_EQ(xpkRemove(xpk, iCycle * 5 + 1), 0);
		}
		ASSERT_EQ(xpkRebuild(xpk), 0);
		ASSERT_EQ(xpkVerifyAll(xpk), 0);
		ASSERT_EQ(xpkSave(xpk), 0);
	}

	ASSERT_EQ(xpkCount(xpk), 5);

	free(pData);
	xpkClose(xpk);
}

int main(int argc, char* argv[]) {
	xrtInit();

	printf("[Corruption Recovery Tests]\n");
	printf("===========================\n\n");

	RUN_TEST(create_valid_package_for_corruption_test);
	RUN_TEST(corrupted_signature_detection);
	RUN_TEST(truncated_file_detection);
	RUN_TEST(damaged_data_recovery_with_verify);
	RUN_TEST(remove_corrupted_file);
	RUN_TEST(rebuild_after_partial_damage);
	RUN_TEST(extract_all_after_removal);
	RUN_TEST(save_and_reload_after_damage);
	RUN_TEST(rebuild_optimizes_structure);
	RUN_TEST(statistics_after_recovery);
	RUN_TEST(traverse_after_recovery);
	RUN_TEST(update_after_recovery);
	RUN_TEST(find_after_recovery);
	RUN_TEST(multiple_recovery_cycles);

	printf("===========================\n");
	printf("All Corruption Recovery Tests Passed!\n");

	return 0;
}
