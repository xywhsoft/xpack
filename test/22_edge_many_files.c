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

TEST(single_file) {
	char* pData = createTestData(1024, 'A');
	const char* sFilename = "test_22_single_file.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, 1024, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 1);

	free(pData);
	xpkClose(xpk);
}

TEST(exactly_256_files) {
	const char* sFilename = "test_22_256_files.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 256; i++ ) {
		char* pData = createTestData(512, 'A' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 512, i), 0);
		free(pData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 256);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 256);
	xpkClose(xpk);
}

TEST(exactly_65536_files) {
	const char* sFilename = "test_22_65536_files.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 65536; i++ ) {
		char* pData = createTestData(256, 'A' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 256, i), 0);
		free(pData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 65536);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 65536);
	xpkClose(xpk);
}

TEST(one_thousand_files) {
	const char* sFilename = "test_22_1000_files.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 1000; i++ ) {
		char* pData = createTestData(512, 'A' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 512, i), 0);
		free(pData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 1000);
	xpkClose(xpk);
}

TEST(ten_thousand_files) {
	const char* sFilename = "test_22_10000_files.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 10000; i++ ) {
		char* pData = createTestData(256, 'A' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 256, i), 0);
		free(pData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 10000);
	xpkClose(xpk);
}

TEST(mixed_size_files) {
	const char* sFilename = "test_22_mixed_sizes.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	int sizes[] = {1, 10, 100, 1024, 10240, 102400, 1048576};
	for ( int i = 0; i < 7; i++ ) {
		char* pData = createTestData(sizes[i], 'A' + i);
		ASSERT_EQ(xpkAppendData(xpk, pData, sizes[i], i + 1), 0);
		free(pData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 7);
	xpkClose(xpk);
}

TEST(files_with_gaps) {
	const char* sFilename = "test_22_gaps.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 10; i++ ) {
		char* pData = createTestData(512, 'A' + i);
		ASSERT_EQ(xpkAppendData(xpk, pData, 512, (i + 1) * 10), 0);
		free(pData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 10);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 10);
	xpkClose(xpk);
}

TEST(sequential_positions) {
	const char* sFilename = "test_22_sequential.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 100; i++ ) {
		char* pData = createTestData(1024, 'A' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 1024, i + 1), 0);
		free(pData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 100);
	xpkClose(xpk);
}

TEST(random_positions) {
	const char* sFilename = "test_22_random_pos.xpk";
	int positions[20];

	for ( int i = 0; i < 20; i++ ) {
		positions[i] = (i * 17) + 1;
	}

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 20; i++ ) {
		char* pData = createTestData(512, 'A' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 512, positions[i]), 0);
		free(pData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 20);
	xpkClose(xpk);
}

TEST(files_with_duplicate_data) {
	const char* sFilename = "test_22_duplicate.xpk";
	char* pData = createTestData(2048, 'D');

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 50; i++ ) {
		ASSERT_EQ(xpkAppendData(xpk, pData, 2048, i + 1), 0);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 50);

	free(pData);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 50; i++ ) {
		char* pExtracted = (char*)malloc(2048);
		int iSize = 2048;
		ASSERT_EQ(xpkExtract(xpk, pExtracted, &iSize, i + 1), 0);
		ASSERT_EQ(iSize, 2048);
		ASSERT_EQ(memcmp(pData, pExtracted, 2048), 0);
		free(pExtracted);
	}

	xpkClose(xpk);
}

TEST(remove_middle_file_from_many) {
	const char* sFilename = "test_22_remove_middle.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 20; i++ ) {
		char* pData = createTestData(512, 'A' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 512, i + 1), 0);
		free(pData);
	}

	ASSERT_EQ(xpkCount(xpk), 20);
	ASSERT_EQ(xpkRemove(xpk, 10), 0);
	ASSERT_EQ(xpkCount(xpk), 19);
	ASSERT_EQ(xpkSave(xpk), 0);

	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 19);
	xpkClose(xpk);
}

TEST(update_many_files) {
	const char* sFilename = "test_22_update_many.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 30; i++ ) {
		char* pData = createTestData(512, 'A' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 512, i + 1), 0);
		free(pData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	for ( int i = 0; i < 30; i++ ) {
		char* pNewData = createTestData(1024, 'X' + (i % 26));
		ASSERT_EQ(xpkUpdateData(xpk, pNewData, 1024, i + 1), 0);
		free(pNewData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 30);
	xpkClose(xpk);
}

TEST(extract_all_from_many_files) {
	const char* sFilename = "test_22_extract_all.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 50; i++ ) {
		char* pData = createTestData(512, 'A' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 512, i + 1), 0);
		free(pData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkExtractAll(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 50);

	xpkClose(xpk);
}

TEST(verify_all_many_files) {
	const char* sFilename = "test_22_verify_all.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 100; i++ ) {
		char* pData = createTestData(256, 'A' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 256, i + 1), 0);
		free(pData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkVerifyAll(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 100);

	xpkClose(xpk);
}

TEST(statistics_on_many_files) {
	const char* sFilename = "test_22_stats_many.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 200; i++ ) {
		char* pData = createTestData(512, 'A' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 512, i + 1), 0);
		free(pData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	int iOriginalSize = 0;
	int iCompressedSize = 0;
	ASSERT_EQ(xpkStatGet(xpk, &iOriginalSize, &iCompressedSize, 0), 0);
	ASSERT_EQ(iOriginalSize, 200 * 512);
	ASSERT_NE(iCompressedSize, 0);

	ASSERT_EQ(xpkCount(xpk), 200);
	xpkClose(xpk);
}

int main(int argc, char* argv[]) {
	xrtInit();

	printf("[Edge Many Files Tests]\n");
	printf("========================\n\n");

	RUN_TEST(single_file);
	RUN_TEST(exactly_256_files);
	RUN_TEST(exactly_65536_files);
	RUN_TEST(one_thousand_files);
	RUN_TEST(ten_thousand_files);
	RUN_TEST(mixed_size_files);
	RUN_TEST(files_with_gaps);
	RUN_TEST(sequential_positions);
	RUN_TEST(random_positions);
	RUN_TEST(files_with_duplicate_data);
	RUN_TEST(remove_middle_file_from_many);
	RUN_TEST(update_many_files);
	RUN_TEST(extract_all_from_many_files);
	RUN_TEST(verify_all_many_files);
	RUN_TEST(statistics_on_many_files);

	printf("========================\n");
	printf("All Edge Many Files Tests Passed!\n");

	return 0;
}
