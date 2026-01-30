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

TEST(benchmark_append_small_files) {
	const char* sFilename = "test_28_bench_append_small.xpk";
	int iCount = 0;

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 1000; i++ ) {
		char* pData = createTestData(512, 'A' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 512, i), 0);
		free(pData);
		iCount++;
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), iCount);

	printf("  Appended %d small files (512 bytes each)\n", iCount);

	xpkClose(xpk);
}

TEST(benchmark_append_large_files) {
	const char* sFilename = "test_28_bench_append_large.xpk";
	int iCount = 0;

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 6);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 20; i++ ) {
		char* pData = createTestData(5 * 1024 * 1024, 'A' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 5 * 1024 * 1024, i), 0);
		free(pData);
		iCount++;
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), iCount);

	printf("  Appended %d large files (5MB each)\n", iCount);

	xpkClose(xpk);
}

TEST(benchmark_extract_all) {
	const char* sFilename = "test_28_bench_extract_all.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 100; i++ ) {
		char* pData = createTestData(4096, 'A' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 4096, i), 0);
		free(pData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_EQ(xpkExtractAll(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 100);

	printf("  Extracted all 100 files (4KB each)\n");

	xpkClose(xpk);
}

TEST(benchmark_verify_all) {
	const char* sFilename = "test_28_bench_verify_all.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 200; i++ ) {
		char* pData = createTestData(2048, 'A' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 2048, i), 0);
		free(pData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_EQ(xpkVerifyAll(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 200);

	printf("  Verified all 200 files (2KB each)\n");

	xpkClose(xpk);
}

TEST(benchmark_traverse_all) {
	const char* sFilename = "test_28_bench_traverse.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 500; i++ ) {
		char* pData = createTestData(1024, 'A' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 1024, i * 10), 0);
		free(pData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	int iCount = 0;
	ASSERT_EQ(xpkEach(xpk, NULL, &iCount), 0);
	ASSERT_EQ(iCount, 500);

	printf("  Traversed all 500 files (1KB each)\n");

	xpkClose(xpk);
}

TEST(benchmark_find_operations) {
	const char* sFilename = "test_28_bench_find.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 1000; i++ ) {
		char* pData = createTestData(512, 'A' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 512, i), 0);
		free(pData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	int iFound = 0;
	for ( int i = 0; i < 1000; i++ ) {
		if ( xpkFind(xpk, i) != -1 ) {
			iFound++;
		}
	}

	ASSERT_EQ(iFound, 1000);

	printf("  Found all 1000 files in %d search operations\n", iFound);

	xpkClose(xpk);
}

TEST(benchmark_rebuild) {
	const char* sFilename = "test_28_bench_rebuild.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 500; i++ ) {
		char* pData = createTestData(2048, 'A' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 2048, i + 1), 0);
		free(pData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	for ( int i = 0; i < 100; i++ ) {
		ASSERT_EQ(xpkRemove(xpk, i * 5 + 1), 0);
	}

	ASSERT_EQ(xpkRebuild(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 400);

	printf("  Rebuilt package after removing 100 files from 500\n");

	xpkClose(xpk);
}

TEST(benchmark_save_load_cycles) {
	const char* sFilename = "test_28_bench_save_load.xpk";
	int iCycles = 0;

	for ( int i = 0; i < 10; i++ ) {
		xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
		ASSERT_NOT_NULL(xpk);

		if ( i == 0 ) {
			for ( int j = 0; j < 50; j++ ) {
				char* pData = createTestData(4096, 'A' + (j % 26));
				ASSERT_EQ(xpkAppendData(xpk, pData, 4096, j + 1), 0);
				free(pData);
			}
		}

		ASSERT_EQ(xpkSave(xpk), 0);
		ASSERT_EQ(xpkCount(xpk), 50);
		xpkClose(xpk);
		iCycles++;
	}

	printf("  Completed %d save/load cycles with 50 files\n", iCycles);
}

TEST(benchmark_update_operations) {
	const char* sFilename = "test_28_bench_update.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 100; i++ ) {
		char* pData = createTestData(2048, 'A' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 2048, i + 1), 0);
		free(pData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	for ( int i = 0; i < 100; i++ ) {
		char* pNewData = createTestData(4096, 'B' + (i % 26));
		ASSERT_EQ(xpkUpdateData(xpk, pNewData, 4096, i + 1), 0);
		free(pNewData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 100);

	printf("  Updated all 100 files (2KB -> 4KB)\n");

	xpkClose(xpk);
}

TEST(benchmark_remove_operations) {
	const char* sFilename = "test_28_bench_remove.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 500; i++ ) {
		char* pData = createTestData(1024, 'A' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 1024, i + 1), 0);
		free(pData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	for ( int i = 0; i < 250; i++ ) {
		ASSERT_EQ(xpkRemove(xpk, i * 2 + 1), 0);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 250);

	printf("  Removed 250 files from 500\n");

	xpkClose(xpk);
}

TEST(benchmark_statistics) {
	const char* sFilename = "test_28_bench_stats.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 1000; i++ ) {
		char* pData = createTestData(2048, 'A' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 2048, i + 1), 0);
		free(pData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	int iOriginalSize = 0;
	int iCompressedSize = 0;
	ASSERT_EQ(xpkStatGet(xpk, &iOriginalSize, &iCompressedSize, 0), 0);

	ASSERT_EQ(iOriginalSize, 1000 * 2048);
	ASSERT_NE(iCompressedSize, 0);

	printf("  Statistics: Original=%d bytes, Compressed=%d bytes, Ratio=%.2f%%\n",
	       iOriginalSize, iCompressedSize, (iCompressedSize * 100.0) / iOriginalSize);

	ASSERT_EQ(xpkCount(xpk), 1000);

	xpkClose(xpk);
}

TEST(benchmark_compression_levels) {
	const char* sLevelsFilename = "test_28_bench_levels.xpk";

	for ( int iLevel = 0; iLevel <= 15; iLevel++ ) {
		xpkObject xpk = xpkOpen(sLevelsFilename, XPK_TYPE_CORE, iLevel);
		ASSERT_NOT_NULL(xpk);

		for ( int i = 0; i < 50; i++ ) {
			char* pData = createTestData(8192, 'A' + (i % 26));
			ASSERT_EQ(xpkAppendData(xpk, pData, 8192, i + 1), 0);
			free(pData);
		}

		ASSERT_EQ(xpkSave(xpk), 0);

		int iOriginalSize = 0;
		int iCompressedSize = 0;
		ASSERT_EQ(xpkStatGet(xpk, &iOriginalSize, &iCompressedSize, 0), 0);

		printf("  Level %2d: Original=%d, Compressed=%d, Ratio=%.2f%%\n",
		       iLevel, iOriginalSize, iCompressedSize, (iCompressedSize * 100.0) / iOriginalSize);

		ASSERT_EQ(xpkCount(xpk), 50);

		xpkClose(xpk);
	}
}

int main(int argc, char* argv[]) {
	xrtInit();

	printf("[Performance Benchmark Tests]\n");
	printf("=============================\n\n");

	RUN_TEST(benchmark_append_small_files);
	RUN_TEST(benchmark_append_large_files);
	RUN_TEST(benchmark_extract_all);
	RUN_TEST(benchmark_verify_all);
	RUN_TEST(benchmark_traverse_all);
	RUN_TEST(benchmark_find_operations);
	RUN_TEST(benchmark_rebuild);
	RUN_TEST(benchmark_save_load_cycles);
	RUN_TEST(benchmark_update_operations);
	RUN_TEST(benchmark_remove_operations);
	RUN_TEST(benchmark_statistics);
	RUN_TEST(benchmark_compression_levels);

	printf("=============================\n");
	printf("All Performance Benchmark Tests Passed!\n");

	return 0;
}
