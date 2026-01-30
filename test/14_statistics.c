/*
 * xPack Ver7 - 统计信息测试
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <xrt/xrt.h>
#include "../src/xpack.h"

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) void test_##name(void)
#define RUN_TEST(name) do { \
    printf("  Testing %s... ", #name); \
    test_##name(); \
    printf("PASSED\n"); \
    tests_passed++; \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAILED\n"); \
        printf("    Assertion failed: %s\n", #cond); \
        printf("    At line %d\n", __LINE__); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_NE(a, b) ASSERT((a) != (b))
#define ASSERT_NULL(a) ASSERT((a) == NULL)
#define ASSERT_NOT_NULL(a) ASSERT((a) != NULL)
#define ASSERT_GT(a, b) ASSERT((a) > (b))
#define ASSERT_GE(a, b) ASSERT((a) >= (b))
#define ASSERT_LT(a, b) ASSERT((a) < (b))

TEST(stat_empty_package) {
	xpkObject xpk = xpkOpen("test_14_stat_empty.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkCount(xpk), 0);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_14_stat_empty.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkCount(xpk), 0);

	uint32_t totalSize = 0;
	uint32_t totalPacked = 0;
	float ratio = 0.0f;

	ASSERT_EQ(xpkStatGet(xpk, &totalSize, &totalPacked, &ratio), 0);
	ASSERT_EQ(totalSize, 0);
	ASSERT_EQ(totalPacked, 0);

	xpkClose(xpk);
}

TEST(stat_single_file) {
	xpkObject xpk = xpkOpen("test_14_stat_single.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Single file content", 19, 6);
	ASSERT_EQ(xpkCount(xpk), 1);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_14_stat_single.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkCount(xpk), 1);

	uint32_t totalSize = 0;
	uint32_t totalPacked = 0;
	float ratio = 0.0f;

	ASSERT_EQ(xpkStatGet(xpk, &totalSize, &totalPacked, &ratio), 0);
	ASSERT_EQ(totalSize, 19);
	ASSERT_GT(totalPacked, 0);
	ASSERT_GT(ratio, 0.0f);
	ASSERT_LT(ratio, 1.0f);

	ASSERT_EQ(totalSize, xpkInfoSize(xpk, 0));
	ASSERT_EQ(totalPacked, xpkInfoPacked(xpk, 0));

	xpkClose(xpk);
}

TEST(stat_multiple_files) {
	xpkObject xpk = xpkOpen("test_14_stat_multiple.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	uint32_t sizes[5] = { 100, 200, 300, 400, 500 };
	uint32_t totalExpected = 1500;

	for (int i = 0; i < 5; i++) {
		char data[512];
		memset(data, 'A' + i, sizes[i]);
		xpkAppendData(xpk, data, sizes[i], 6);
	}

	ASSERT_EQ(xpkCount(xpk), 5);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_14_stat_multiple.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkCount(xpk), 5);

	uint32_t totalSize = 0;
	uint32_t totalPacked = 0;
	float ratio = 0.0f;

	ASSERT_EQ(xpkStatGet(xpk, &totalSize, &totalPacked, &ratio), 0);
	ASSERT_EQ(totalSize, totalExpected);

	uint32_t sumSizes = 0;
	uint32_t sumPacked = 0;
	for (int i = 0; i < 5; i++) {
		sumSizes += xpkInfoSize(xpk, i);
		sumPacked += xpkInfoPacked(xpk, i);
	}

	ASSERT_EQ(totalSize, sumSizes);
	ASSERT_EQ(totalPacked, sumPacked);

	ASSERT_GT(totalPacked, 0);
	ASSERT_LT(totalPacked, totalSize);

	ASSERT_GT(ratio, 0.0f);
	ASSERT_LT(ratio, 1.0f);

	xpkClose(xpk);
}

TEST(stat_compression_levels) {
	for (int level = 0; level <= 15; level++) {
		char filename[64];
		sprintf(filename, "test_14_stat_level_%d.xpk", level);

		xpkObject xpk = xpkOpen(filename, 0, 0);
		ASSERT_NOT_NULL(xpk);

		char data[10240];
		memset(data, 'A', sizeof(data));

		for (int i = 0; i < 10; i++) {
			xpkAppendData(xpk, data, sizeof(data), level);
		}

		ASSERT_EQ(xpkSave(xpk), 0);
		xpkClose(xpk);

		xpk = xpkOpen(filename, 0, 1);
		ASSERT_NOT_NULL(xpk);

		ASSERT_EQ(xpkCount(xpk), 10);

		uint32_t totalSize = 0;
		uint32_t totalPacked = 0;
		float ratio = 0.0f;

		ASSERT_EQ(xpkStatGet(xpk, &totalSize, &totalPacked, &ratio), 0);
		ASSERT_EQ(totalSize, 102400);
		ASSERT_GT(totalPacked, 0);

		ASSERT_GT(ratio, 0.0f);
		ASSERT_LE(ratio, 1.0f);

		if (level > 0) {
			ASSERT_LT(ratio, 1.0f);
		}

		xpkClose(xpk);
	}
}

TEST(stat_empty_files) {
	xpkObject xpk = xpkOpen("test_14_stat_empty_files.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 5; i++) {
		xpkAppendData(xpk, NULL, 0, 6);
	}

	ASSERT_EQ(xpkCount(xpk), 5);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_14_stat_empty_files.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkCount(xpk), 5);

	uint32_t totalSize = 0;
	uint32_t totalPacked = 0;
	float ratio = 0.0f;

	ASSERT_EQ(xpkStatGet(xpk, &totalSize, &totalPacked, &ratio), 0);
	ASSERT_EQ(totalSize, 0);
	ASSERT_EQ(totalPacked, 0);
	ASSERT_EQ(ratio, 0.0f);

	xpkClose(xpk);
}

TEST(stat_mixed_sizes) {
	xpkObject xpk = xpkOpen("test_14_stat_mixed.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	uint32_t sizes[] = { 0, 1, 10, 100, 1000, 10000, 100000 };

	for (int i = 0; i < 7; i++) {
		char* data = malloc(sizes[i]);
		ASSERT_NOT_NULL(data);
		memset(data, 'A' + i, sizes[i]);
		xpkAppendData(xpk, data, sizes[i], 6);
		free(data);
	}

	ASSERT_EQ(xpkCount(xpk), 7);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_14_stat_mixed.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkCount(xpk), 7);

	uint32_t totalSize = 0;
	uint32_t totalPacked = 0;
	float ratio = 0.0f;

	ASSERT_EQ(xpkStatGet(xpk, &totalSize, &totalPacked, &ratio), 0);

	uint32_t expectedSize = 0;
	for (int i = 0; i < 7; i++) {
		expectedSize += sizes[i];
	}

	ASSERT_EQ(totalSize, expectedSize);
	ASSERT_GT(totalPacked, 0);
	ASSERT_LT(totalPacked, totalSize);
	ASSERT_GT(ratio, 0.0f);
	ASSERT_LT(ratio, 1.0f);

	xpkClose(xpk);
}

TEST(stat_after_remove) {
	xpkObject xpk = xpkOpen("test_14_stat_remove.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 10; i++) {
		char data[100];
		sprintf(data, "File %d content", i);
		xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
	}

	ASSERT_EQ(xpkCount(xpk), 10);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_14_stat_remove.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkCount(xpk), 10);

	uint32_t totalSize1 = 0;
	uint32_t totalPacked1 = 0;
	float ratio1 = 0.0f;

	ASSERT_EQ(xpkStatGet(xpk, &totalSize1, &totalPacked1, &ratio1), 0);

	ASSERT_EQ(xpkRemove(xpk, 2), 0);
	ASSERT_EQ(xpkRemove(xpk, 5), 0);
	ASSERT_EQ(xpkRemove(xpk, 7), 0);

	ASSERT_EQ(xpkCount(xpk), 7);

	uint32_t totalSize2 = 0;
	uint32_t totalPacked2 = 0;
	float ratio2 = 0.0f;

	ASSERT_EQ(xpkStatGet(xpk, &totalSize2, &totalPacked2, &ratio2), 0);

	ASSERT_LT(totalSize2, totalSize1);
	ASSERT_LT(totalPacked2, totalPacked1);

	xpkClose(xpk);
}

TEST(stat_after_update) {
	xpkObject xpk = xpkOpen("test_14_stat_update.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 5; i++) {
		char data[100];
		sprintf(data, "Original %d", i);
		xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
	}

	ASSERT_EQ(xpkCount(xpk), 5);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_14_stat_update.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkCount(xpk), 5);

	uint32_t totalSize1 = 0;
	uint32_t totalPacked1 = 0;
	float ratio1 = 0.0f;

	ASSERT_EQ(xpkStatGet(xpk, &totalSize1, &totalPacked1, &ratio1), 0);

	for (int i = 0; i < 5; i++) {
		char data[200];
		sprintf(data, "Updated file %d with more content", i);
		xpkUpdateData(xpk, i, data, (uint32_t)strlen(data), 6);
	}

	uint32_t totalSize2 = 0;
	uint32_t totalPacked2 = 0;
	float ratio2 = 0.0f;

	ASSERT_EQ(xpkStatGet(xpk, &totalSize2, &totalPacked2, &ratio2), 0);

	ASSERT_GT(totalSize2, totalSize1);
	ASSERT_GT(totalPacked2, totalPacked1);

	xpkClose(xpk);
}

TEST(stat_no_compression) {
	xpkObject xpk = xpkOpen("test_14_stat_nocomp.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	char data[10000];
	memset(data, 'A', sizeof(data));

	for (int i = 0; i < 10; i++) {
		xpkAppendData(xpk, data, sizeof(data), 0);
	}

	ASSERT_EQ(xpkCount(xpk), 10);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_14_stat_nocomp.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkCount(xpk), 10);

	uint32_t totalSize = 0;
	uint32_t totalPacked = 0;
	float ratio = 0.0f;

	ASSERT_EQ(xpkStatGet(xpk, &totalSize, &totalPacked, &ratio), 0);
	ASSERT_EQ(totalSize, 100000);
	ASSERT_EQ(totalPacked, 100000);
	ASSERT_EQ(ratio, 1.0f);

	xpkClose(xpk);
}

TEST(stat_high_compression) {
	xpkObject xpk = xpkOpen("test_14_stat_high.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 100000;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);
	memset(data, 'A', size);

	for (int i = 0; i < 10; i++) {
		xpkAppendData(xpk, data, (uint32_t)size, 15);
	}

	free(data);

	ASSERT_EQ(xpkCount(xpk), 10);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_14_stat_high.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkCount(xpk), 10);

	uint32_t totalSize = 0;
	uint32_t totalPacked = 0;
	float ratio = 0.0f;

	ASSERT_EQ(xpkStatGet(xpk, &totalSize, &totalPacked, &ratio), 0);
	ASSERT_EQ(totalSize, 1000000);

	ASSERT_GT(totalPacked, 0);
	ASSERT_LT(totalPacked, totalSize);
	ASSERT_LT(ratio, 0.1f);

	xpkClose(xpk);
}

TEST(stat_path_mode) {
	xpkObject xpk = xpkOpen("test_14_stat_path.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	xpkPathAppendData(xpk, "file1.txt", "Content 1", 9, 6);
	xpkPathAppendData(xpk, "file2.txt", "Content 2", 9, 6);
	xpkPathAppendData(xpk, "file3.txt", "Content 3", 9, 6);

	ASSERT_EQ(xpkCount(xpk), 3);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_14_stat_path.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkCount(xpk), 3);

	uint32_t totalSize = 0;
	uint32_t totalPacked = 0;
	float ratio = 0.0f;

	ASSERT_EQ(xpkStatGet(xpk, &totalSize, &totalPacked, &ratio), 0);
	ASSERT_EQ(totalSize, 27);
	ASSERT_GT(totalPacked, 0);
	ASSERT_GT(ratio, 0.0f);
	ASSERT_LT(ratio, 1.0f);

	xpkClose(xpk);
}

TEST(stat_index_mode) {
	xpkObject xpk = xpkOpen("test_14_stat_index.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

	xpkIndexAppendData(xpk, 10, "Data 1", 6, 6);
	xpkIndexAppendData(xpk, 20, "Data 2", 6, 6);
	xpkIndexAppendData(xpk, 30, "Data 3", 6, 6);

	ASSERT_EQ(xpkCount(xpk), 3);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_14_stat_index.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkCount(xpk), 3);

	uint32_t totalSize = 0;
	uint32_t totalPacked = 0;
	float ratio = 0.0f;

	ASSERT_EQ(xpkStatGet(xpk, &totalSize, &totalPacked, &ratio), 0);
	ASSERT_EQ(totalSize, 18);
	ASSERT_GT(totalPacked, 0);
	ASSERT_GT(ratio, 0.0f);
	ASSERT_LT(ratio, 1.0f);

	xpkClose(xpk);
}

int main(void) {
	xrtInit();

	printf("=================================================\n");
	printf("  xPack Ver7 - Statistics Test\n");
	printf("=================================================\n\n");

	printf("[Basic Statistics Tests]\n");
	RUN_TEST(stat_empty_package);
	RUN_TEST(stat_single_file);
	RUN_TEST(stat_multiple_files);
	printf("\n");

	printf("[Compression Level Tests]\n");
	RUN_TEST(stat_compression_levels);
	printf("\n");

	printf("[Edge Cases Tests]\n");
	RUN_TEST(stat_empty_files);
	RUN_TEST(stat_mixed_sizes);
	printf("\n");

	printf("[Operation After Statistics Tests]\n");
	RUN_TEST(stat_after_remove);
	RUN_TEST(stat_after_update);
	printf("\n");

	printf("[Compression Ratio Tests]\n");
	RUN_TEST(stat_no_compression);
	RUN_TEST(stat_high_compression);
	printf("\n");

	printf("[Mode Specific Tests]\n");
	RUN_TEST(stat_path_mode);
	RUN_TEST(stat_index_mode);
	printf("\n");

	printf("=================================================\n");
	printf("  Results: %d passed, %d failed\n", tests_passed, tests_failed);
	printf("=================================================\n");

	return tests_failed > 0 ? 1 : 0;
}
