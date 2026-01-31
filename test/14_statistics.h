/*
 * xPack Ver7 - statistics (14)
 */

#include "test_framework.h"

TEST(stats_basic) {
	xpkObject xpk = xpkOpen("test_14_stats_basic.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	char data[1024];
	memset(data, 'A', sizeof(data));
	xpkAppendData(xpk, data, sizeof(data), 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_14_stats_basic.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	xpkStat stat;
	ASSERT_EQ(xpkStatGet(xpk, &stat), 0);
	ASSERT_EQ(stat.fileCount, 1);
	ASSERT_GT(stat.totalSize, 0);
	ASSERT_GT(stat.packedSize, 0);
	ASSERT_GT(stat.ratio, 0.0);

	xpkClose(xpk);
}

TEST(stats_multiple_files) {
	xpkObject xpk = xpkOpen("test_14_stats_multi.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 10; i++) {
		char data[512];
		memset(data, 'A' + i, sizeof(data));
		xpkAppendData(xpk, data, sizeof(data), 6);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_14_stats_multi.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	xpkStat stat;
	ASSERT_EQ(xpkStatGet(xpk, &stat), 0);
	ASSERT_EQ(stat.fileCount, 10);
	ASSERT_GT(stat.totalSize, 0);
	ASSERT_GT(stat.packedSize, 0);
	ASSERT_GT(stat.ratio, 0.0);
	ASSERT_GT(stat.totalSize, stat.packedSize);

	xpkClose(xpk);
}

TEST(stats_empty_package) {
	xpkObject xpk = xpkOpen("test_14_stats_empty.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_14_stats_empty.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	xpkStat stat;
	ASSERT_EQ(xpkStatGet(xpk, &stat), 0);
	ASSERT_EQ(stat.fileCount, 0);
	ASSERT_EQ(stat.totalSize, 0);
	ASSERT_EQ(stat.packedSize, 0);

	xpkClose(xpk);
}

TEST(stats_compression_levels) {
	xpkObject xpk = xpkOpen("test_14_stats_levels.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int level = 0; level <= 9; level++) {
		char data[1024];
		memset(data, 'X', sizeof(data));
		xpkAppendData(xpk, data, sizeof(data), level);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_14_stats_levels.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	xpkStat stat;
	ASSERT_EQ(xpkStatGet(xpk, &stat), 0);
	ASSERT_EQ(stat.fileCount, 10);
	ASSERT_GT(stat.totalSize, 0);
	ASSERT_GT(stat.packedSize, 0);
	ASSERT_GT(stat.ratio, 0.0);

	xpkClose(xpk);
}

TEST(stats_after_removal) {
	xpkObject xpk = xpkOpen("test_14_stats_removal.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 20; i++) {
		char data[256];
		sprintf(data, "File %d", i);
		xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 20);
	xpkClose(xpk);

	xpk = xpkOpen("test_14_stats_removal.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 10; i++) {
		ASSERT_EQ(xpkRemove(xpk, 0), 0);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_14_stats_removal.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	xpkStat stat;
	ASSERT_EQ(xpkStatGet(xpk, &stat), 0);
	ASSERT_EQ(stat.fileCount, 10);
	ASSERT_GT(stat.totalSize, 0);

	xpkClose(xpk);
}

TEST(stats_large_files) {
	xpkObject xpk = xpkOpen("test_14_stats_large.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 5; i++) {
		char* data = (char*)malloc(10 * 1024 * 1024);
		memset(data, 'A' + i, 10 * 1024 * 1024);
		ASSERT_NE(xpkAppendData(xpk, data, 10 * 1024 * 1024, 6), UINT32_MAX);
		free(data);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_14_stats_large.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	xpkStat stat;
	ASSERT_EQ(xpkStatGet(xpk, &stat), 0);
	ASSERT_EQ(stat.fileCount, 5);
	ASSERT_GT(stat.totalSize, 0);
	ASSERT_GT(stat.packedSize, 0);
	ASSERT_GT(stat.ratio, 0.0);

	xpkClose(xpk);
}

TEST(stats_empty_files) {
	xpkObject xpk = xpkOpen("test_14_stats_empty_files.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 5; i++) {
		ASSERT_NE(xpkAppendData(xpk, NULL, 0, 6), UINT32_MAX);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 5);
	xpkClose(xpk);

	xpk = xpkOpen("test_14_stats_empty_files.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	xpkStat stat;
	ASSERT_EQ(xpkStatGet(xpk, &stat), 0);
	ASSERT_EQ(stat.fileCount, 5);
	ASSERT_EQ(stat.totalSize, 0);
	ASSERT_EQ(stat.packedSize, 0);

	xpkClose(xpk);
}

TEST(stats_mixed_sizes) {
	xpkObject xpk = xpkOpen("test_14_stats_mixed.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	char* data1 = (char*)malloc(1024);
	memset(data1, 'A', 1024);
	ASSERT_NE(xpkAppendData(xpk, data1, 1024, 6), UINT32_MAX);
	free(data1);

	char* data2 = (char*)malloc(10 * 1024);
	memset(data2, 'B', 10 * 1024);
	ASSERT_NE(xpkAppendData(xpk, data2, 10 * 1024, 6), UINT32_MAX);
	free(data2);

	char* data3 = (char*)malloc(100 * 1024);
	memset(data3, 'C', 100 * 1024);
	ASSERT_NE(xpkAppendData(xpk, data3, 100 * 1024, 6), UINT32_MAX);
	free(data3);

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 3);
	xpkClose(xpk);

	xpk = xpkOpen("test_14_stats_mixed.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	xpkStat stat;
	ASSERT_EQ(xpkStatGet(xpk, &stat), 0);
	ASSERT_EQ(stat.fileCount, 3);
	ASSERT_EQ(stat.totalSize, 1024 + 10 * 1024 + 100 * 1024);
	ASSERT_GT(stat.packedSize, 0);
	ASSERT_GT(stat.ratio, 0.0);

	xpkClose(xpk);
}

TEST(stats_after_update) {
	xpkObject xpk = xpkOpen("test_14_stats_update.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 5; i++) {
		char data[512];
		sprintf(data, "Original file %d", i);
		ASSERT_NE(xpkAppendData(xpk, data, (uint32_t)strlen(data), 6), UINT32_MAX);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 5);
	xpkClose(xpk);

	xpk = xpkOpen("test_14_stats_update.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 5; i++) {
		char newData[2048];
		sprintf(newData, "Updated file %d with much more data", i);
		ASSERT_EQ(xpkUpdateData(xpk, i, newData, (uint32_t)strlen(newData), 6), 0);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_14_stats_update.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	xpkStat stat;
	ASSERT_EQ(xpkStatGet(xpk, &stat), 0);
	ASSERT_EQ(stat.fileCount, 5);
	ASSERT_GT(stat.totalSize, 5 * 512);
	ASSERT_GT(stat.packedSize, 0);
	ASSERT_GT(stat.ratio, 0.0);

	xpkClose(xpk);
}

TEST(stats_no_compression) {
	xpkObject xpk = xpkOpen("test_14_stats_no_comp.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 5; i++) {
		char data[1024];
		memset(data, 'A' + i, 1024);
		ASSERT_NE(xpkAppendData(xpk, data, 1024, 0), UINT32_MAX);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 5);
	xpkClose(xpk);

	xpk = xpkOpen("test_14_stats_no_comp.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	xpkStat stat;
	ASSERT_EQ(xpkStatGet(xpk, &stat), 0);
	ASSERT_EQ(stat.fileCount, 5);
	ASSERT_GT(stat.totalSize, 0);
	ASSERT_GT(stat.packedSize, 0);

	ASSERT_EQ(xpkInfoSize(xpk, 0), 1024);
	ASSERT_EQ(xpkInfoPacked(xpk, 0), 1024);

	xpkClose(xpk);
}

TEST(stats_high_compression) {
	xpkObject xpk = xpkOpen("test_14_stats_high_comp.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 5; i++) {
		char* data = (char*)malloc(100 * 1024);
		memset(data, 'A' + i, 100 * 1024);
		ASSERT_NE(xpkAppendData(xpk, data, 100 * 1024, 9), UINT32_MAX);
		free(data);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 5);
	xpkClose(xpk);

	xpk = xpkOpen("test_14_stats_high_comp.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	xpkStat stat;
	ASSERT_EQ(xpkStatGet(xpk, &stat), 0);
	ASSERT_EQ(stat.fileCount, 5);
	ASSERT_GT(stat.totalSize, 0);
	ASSERT_GT(stat.packedSize, 0);
	ASSERT_GT(stat.ratio, 0.0);

	ASSERT_GT(stat.totalSize, stat.packedSize);

	xpkClose(xpk);
}

TEST(stats_path_mode) {
	xpkObject xpk = xpkOpen("test_14_stats_path.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	for (int i = 0; i < 5; i++) {
		char path[64];
		sprintf(path, "files/file%d.txt", i);
		char data[1024];
		memset(data, 'A' + i, 1024);
		ASSERT_EQ(xpkPathAppendData(xpk, path, data, 1024, 6), 0);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 5);
	xpkClose(xpk);

	xpk = xpkOpen("test_14_stats_path.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	xpkStat stat;
	ASSERT_EQ(xpkStatGet(xpk, &stat), 0);
	ASSERT_EQ(stat.fileCount, 5);
	ASSERT_GT(stat.totalSize, 0);
	ASSERT_GT(stat.packedSize, 0);
	ASSERT_GT(stat.ratio, 0.0);

	xpkClose(xpk);
}

TEST(stats_index_mode) {
	xpkObject xpk = xpkOpen("test_14_stats_index.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

	for (int i = 0; i < 5; i++) {
		char data[1024];
		memset(data, 'A' + i, 1024);
		ASSERT_EQ(xpkIndexAppendData(xpk, 100 + i, data, 1024, 6), 0);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 5);
	xpkClose(xpk);

	xpk = xpkOpen("test_14_stats_index.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	xpkStat stat;
	ASSERT_EQ(xpkStatGet(xpk, &stat), 0);
	ASSERT_EQ(stat.fileCount, 5);
	ASSERT_GT(stat.totalSize, 0);
	ASSERT_GT(stat.packedSize, 0);
	ASSERT_GT(stat.ratio, 0.0);

	xpkClose(xpk);
}

void register_14_statistics_tests(void) {
	TEST_REGISTER(stats_basic, CAT_STATS, "Test basic statistics");
	TEST_REGISTER(stats_multiple_files, CAT_STATS, "Test statistics with multiple files");
	TEST_REGISTER(stats_empty_package, CAT_STATS, "Test statistics with empty package");
	TEST_REGISTER(stats_compression_levels, CAT_STATS, "Test statistics with compression levels");
	TEST_REGISTER(stats_after_removal, CAT_STATS, "Test statistics after removal");
	TEST_REGISTER(stats_large_files, CAT_STATS, "Test statistics with large files");
	TEST_REGISTER(stats_empty_files, CAT_STATS, "Test statistics with empty files");
	TEST_REGISTER(stats_mixed_sizes, CAT_STATS, "Test statistics with mixed file sizes");
	TEST_REGISTER(stats_after_update, CAT_STATS, "Test statistics after update");
	TEST_REGISTER(stats_no_compression, CAT_STATS, "Test statistics with no compression");
	TEST_REGISTER(stats_high_compression, CAT_STATS, "Test statistics with high compression");
	TEST_REGISTER(stats_path_mode, CAT_STATS, "Test statistics in path mode");
	TEST_REGISTER(stats_index_mode, CAT_STATS, "Test statistics in index mode");
}
