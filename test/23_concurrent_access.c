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

TEST(rapid_open_close_cycles) {
	char* pData = createTestData(1024, 'A');
	const char* sFilename = "test_23_rapid_open.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, 1024, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	for ( int i = 0; i < 100; i++ ) {
		xpk = xpkOpen(sFilename, 0, 0);
		ASSERT_NOT_NULL(xpk);
		ASSERT_EQ(xpkCount(xpk), 1);
		xpkClose(xpk);
	}

	free(pData);
}

TEST(read_only_while_another_writes) {
	char* pData = createTestData(2048, 'B');
	const char* sFilename = "test_23_readonly_write.xpk";

	xpkObject xpkWrite = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpkWrite);
	ASSERT_EQ(xpkAppendData(xpkWrite, pData, 2048, 1), 0);
	ASSERT_EQ(xpkSave(xpkWrite), 0);

	xpkObject xpkRead = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpkRead);
	ASSERT_EQ(xpkCount(xpkRead), 1);

	ASSERT_EQ(xpkAppendData(xpkWrite, pData, 2048, 2), 0);
	ASSERT_EQ(xpkSave(xpkWrite), 0);
	ASSERT_EQ(xpkCount(xpkWrite), 2);

	ASSERT_EQ(xpkCount(xpkRead), 2);

	free(pData);
	xpkClose(xpkWrite);
	xpkClose(xpkRead);
}

TEST(multiple_sequential_operations) {
	char* pData = createTestData(512, 'C');
	const char* sFilename = "test_23_sequential.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 50; i++ ) {
		ASSERT_EQ(xpkAppendData(xpk, pData, 512, i + 1), 0);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	for ( int i = 0; i < 50; i++ ) {
		char* pExtracted = (char*)malloc(512);
		int iSize = 512;
		ASSERT_EQ(xpkExtract(xpk, pExtracted, &iSize, i + 1), 0);
		ASSERT_EQ(iSize, 512);
		ASSERT_EQ(memcmp(pData, pExtracted, 512), 0);
		free(pExtracted);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	for ( int i = 0; i < 50; i++ ) {
		ASSERT_EQ(xpkRemove(xpk, i + 1), 0);
	}

	ASSERT_EQ(xpkCount(xpk), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	free(pData);
	xpkClose(xpk);
}

TEST(alternate_read_write) {
	char* pData = createTestData(1024, 'D');
	const char* sFilename = "test_23_alternate.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 20; i++ ) {
		ASSERT_EQ(xpkAppendData(xpk, pData, 1024, i + 1), 0);

		char* pExtracted = (char*)malloc(1024);
		int iSize = 1024;
		ASSERT_EQ(xpkExtract(xpk, pExtracted, &iSize, i + 1), 0);
		ASSERT_EQ(iSize, 1024);
		ASSERT_EQ(memcmp(pData, pExtracted, 1024), 0);
		free(pExtracted);
	}

	ASSERT_EQ(xpkCount(xpk), 20);
	ASSERT_EQ(xpkSave(xpk), 0);

	free(pData);
	xpkClose(xpk);
}

TEST(verify_during_operations) {
	char* pData = createTestData(1536, 'E');
	const char* sFilename = "test_23_verify_during.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 30; i++ ) {
		ASSERT_EQ(xpkAppendData(xpk, pData, 1536, i + 1), 0);

		if ( i > 0 ) {
			ASSERT_EQ(xpkVerify(xpk, i), 0);
		}
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	for ( int i = 0; i < 30; i++ ) {
		ASSERT_EQ(xpkVerify(xpk, i + 1), 0);
	}

	ASSERT_EQ(xpkCount(xpk), 30);
	ASSERT_EQ(xpkSave(xpk), 0);

	free(pData);
	xpkClose(xpk);
}

TEST(traverse_during_modifications) {
	char* pData = createTestData(1024, 'F');
	const char* sFilename = "test_23_traverse_during.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 20; i++ ) {
		ASSERT_EQ(xpkAppendData(xpk, pData, 1024, i * 10), 0);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	int iCount = 0;
	ASSERT_EQ(xpkEach(xpk, NULL, &iCount), 0);
	ASSERT_EQ(iCount, 20);

	ASSERT_EQ(xpkAppendData(xpk, pData, 1024, 200), 0);

	iCount = 0;
	ASSERT_EQ(xpkEach(xpk, NULL, &iCount), 0);
	ASSERT_EQ(iCount, 21);

	ASSERT_EQ(xpkRemove(xpk, 100), 0);

	iCount = 0;
	ASSERT_EQ(xpkEach(xpk, NULL, &iCount), 0);
	ASSERT_EQ(iCount, 20);

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 20);

	free(pData);
	xpkClose(xpk);
}

TEST(statistics_during_operations) {
	char* pData = createTestData(2048, 'G');
	const char* sFilename = "test_23_stats_during.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	int iOriginalSize = 0;
	int iCompressedSize = 0;

	ASSERT_EQ(xpkStatGet(xpk, &iOriginalSize, &iCompressedSize, 0), 0);
	ASSERT_EQ(iOriginalSize, 0);

	for ( int i = 0; i < 10; i++ ) {
		ASSERT_EQ(xpkAppendData(xpk, pData, 2048, i + 1), 0);

		iOriginalSize = 0;
		iCompressedSize = 0;
		ASSERT_EQ(xpkStatGet(xpk, &iOriginalSize, &iCompressedSize, 0), 0);
		ASSERT_EQ(iOriginalSize, (i + 1) * 2048);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_EQ(xpkRemove(xpk, 5), 0);

	iOriginalSize = 0;
	iCompressedSize = 0;
	ASSERT_EQ(xpkStatGet(xpk, &iOriginalSize, &iCompressedSize, 0), 0);
	ASSERT_EQ(iOriginalSize, 9 * 2048);

	ASSERT_EQ(xpkSave(xpk), 0);

	free(pData);
	xpkClose(xpk);
}

TEST(rapid_append_extract_cycle) {
	char* pData = createTestData(512, 'H');
	const char* sFilename = "test_23_append_extract.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 100; i++ ) {
		ASSERT_EQ(xpkAppendData(xpk, pData, 512, i + 1), 0);

		char* pExtracted = (char*)malloc(512);
		int iSize = 512;
		ASSERT_EQ(xpkExtract(xpk, pExtracted, &iSize, i + 1), 0);
		ASSERT_EQ(iSize, 512);
		ASSERT_EQ(memcmp(pData, pExtracted, 512), 0);
		free(pExtracted);
	}

	ASSERT_EQ(xpkCount(xpk), 100);
	ASSERT_EQ(xpkSave(xpk), 0);

	free(pData);
	xpkClose(xpk);
}

TEST(update_during_operations) {
	char* pData = createTestData(1024, 'I');
	char* pNewData = createTestData(2048, 'J');
	const char* sFilename = "test_23_update_during.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 20; i++ ) {
		ASSERT_EQ(xpkAppendData(xpk, pData, 1024, i + 1), 0);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	for ( int i = 0; i < 10; i++ ) {
		ASSERT_EQ(xpkUpdateData(xpk, pNewData, 2048, i + 1), 0);

		char* pExtracted = (char*)malloc(2048);
		int iSize = 2048;
		ASSERT_EQ(xpkExtract(xpk, pExtracted, &iSize, i + 1), 0);
		ASSERT_EQ(iSize, 2048);
		ASSERT_EQ(memcmp(pNewData, pExtracted, 2048), 0);
		free(pExtracted);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 20);

	free(pData);
	free(pNewData);
	xpkClose(xpk);
}

TEST(multiple_save_cycles) {
	char* pData = createTestData(1536, 'K');
	const char* sFilename = "test_23_multiple_save.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 50; i++ ) {
		ASSERT_EQ(xpkAppendData(xpk, pData, 1536, i + 1), 0);

		if ( i % 10 == 0 ) {
			ASSERT_EQ(xpkSave(xpk), 0);
		}
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 50);

	for ( int i = 0; i < 10; i++ ) {
		ASSERT_EQ(xpkRemove(xpk, i + 1), 0);
		ASSERT_EQ(xpkSave(xpk), 0);
	}

	ASSERT_EQ(xpkCount(xpk), 40);
	ASSERT_EQ(xpkSave(xpk), 0);

	free(pData);
	xpkClose(xpk);
}

TEST(find_during_modifications) {
	char* pData = createTestData(1024, 'L');
	const char* sFilename = "test_23_find_during.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 5);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppendData(xpk, pData, 1024, 100), 0);
	ASSERT_EQ(xpkAppendData(xpk, pData, 1024, 200), 0);
	ASSERT_EQ(xpkAppendData(xpk, pData, 1024, 300), 0);

	ASSERT_EQ(xpkFind(xpk, 200), 200);

	ASSERT_EQ(xpkRemove(xpk, 200), 0);
	ASSERT_EQ(xpkFind(xpk, 200), -1);

	ASSERT_EQ(xpkAppendData(xpk, pData, 1024, 200), 0);
	ASSERT_EQ(xpkFind(xpk, 200), 200);

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 3);

	free(pData);
	xpkClose(xpk);
}

TEST(info_during_operations) {
	char* pData = createTestData(2048, 'M');
	const char* sFilename = "test_23_info_during.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 10; i++ ) {
		ASSERT_EQ(xpkAppendData(xpk, pData, 2048, i + 1), 0);

		int iOriginalSize = 0;
		int iCompressedSize = 0;
		ASSERT_EQ(xpkInfo(xpk, &iOriginalSize, &iCompressedSize, 0, i + 1), 0);
		ASSERT_EQ(iOriginalSize, 2048);
		ASSERT_NE(iCompressedSize, 0);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	for ( int i = 0; i < 5; i++ ) {
		ASSERT_EQ(xpkRemove(xpk, i + 1), 0);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 5);

	free(pData);
	xpkClose(xpk);
}

TEST(rebuild_during_operations) {
	char* pData = createTestData(1024, 'N');
	const char* sFilename = "test_23_rebuild_during.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 20; i++ ) {
		ASSERT_EQ(xpkAppendData(xpk, pData, 1024, i + 1), 0);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkRebuild(xpk), 0);

	ASSERT_EQ(xpkRemove(xpk, 5), 0);
	ASSERT_EQ(xpkRemove(xpk, 10), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkRebuild(xpk), 0);

	ASSERT_EQ(xpkCount(xpk), 18);
	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	free(pData);
	xpkClose(xpk);
}

int main(int argc, char* argv[]) {
	xrtInit();

	printf("[Concurrent Access Tests]\n");
	printf("=========================\n\n");

	RUN_TEST(rapid_open_close_cycles);
	RUN_TEST(read_only_while_another_writes);
	RUN_TEST(multiple_sequential_operations);
	RUN_TEST(alternate_read_write);
	RUN_TEST(verify_during_operations);
	RUN_TEST(traverse_during_modifications);
	RUN_TEST(statistics_during_operations);
	RUN_TEST(rapid_append_extract_cycle);
	RUN_TEST(update_during_operations);
	RUN_TEST(multiple_save_cycles);
	RUN_TEST(find_during_operations);
	RUN_TEST(info_during_operations);
	RUN_TEST(rebuild_during_operations);

	printf("=========================\n");
	printf("All Concurrent Access Tests Passed!\n");

	return 0;
}
