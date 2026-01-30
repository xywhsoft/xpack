/*
 * xPack Ver7 - 重建功能测试
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
#define ASSERT_LT(a, b) ASSERT((a) < (b))

TEST(rebuild_empty_package) {
	xpkObject xpk = xpkOpen("test_16_rebuild_empty.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_16_rebuild_empty.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkRebuild(xpk), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_16_rebuild_empty.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 0);
	xpkClose(xpk);
}

TEST(rebuild_single_file) {
	xpkObject xpk = xpkOpen("test_16_rebuild_single.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Single file data", 16, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_16_rebuild_single.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkRebuild(xpk), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_16_rebuild_single.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 1);

	uint32_t outSize = 0;
	void* data = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(data);
	ASSERT_EQ(outSize, 16);
	ASSERT_EQ(memcmp(data, "Single file data", 16), 0);
	xpkFree(data);

	xpkClose(xpk);
}

TEST(rebuild_multiple_files) {
	xpkObject xpk = xpkOpen("test_16_rebuild_multiple.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	uint32_t hashes[10];
	size_t sizes[10];

	for (int i = 0; i < 10; i++) {
		sizes[i] = (i + 1) * 200;
		void* data = malloc(sizes[i]);
		ASSERT_NOT_NULL(data);
		memset(data, 'A' + i, sizes[i]);

		hashes[i] = xrtHash32((ptr)data, sizes[i]);

		xpkAppendData(xpk, data, (uint32_t)sizes[i], 6);

		ASSERT_EQ(xpkInfoHash(xpk, i), hashes[i]);

		free(data);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_16_rebuild_multiple.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkRebuild(xpk), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_16_rebuild_multiple.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 10);

	for (int i = 0; i < 10; i++) {
		ASSERT_EQ(xpkInfoHash(xpk, i), hashes[i]);

		void* original = malloc(sizes[i]);
		ASSERT_NOT_NULL(original);
		memset(original, 'A' + i, sizes[i]);

		uint32_t outSize = 0;
		void* extracted = xpkExtractData(xpk, i, &outSize);
		ASSERT_NOT_NULL(extracted);
		ASSERT_EQ(outSize, (uint32_t)sizes[i]);
		ASSERT_EQ(memcmp(original, extracted, sizes[i]), 0);

		free(original);
		xpkFree(extracted);
	}

	xpkClose(xpk);
}

TEST(rebuild_after_remove) {
	xpkObject xpk = xpkOpen("test_16_rebuild_remove.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 10; i++) {
		char data[100];
		sprintf(data, "File %d content", i);
		xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
	}

	ASSERT_EQ(xpkCount(xpk), 10);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_16_rebuild_remove.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkRemove(xpk, 2), 0);
	ASSERT_EQ(xpkRemove(xpk, 5), 0);
	ASSERT_EQ(xpkRemove(xpk, 7), 0);

	ASSERT_EQ(xpkCount(xpk), 7);

	ASSERT_EQ(xpkRebuild(xpk), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_16_rebuild_remove.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 7);

	xpkClose(xpk);
}

TEST(rebuild_after_update) {
	xpkObject xpk = xpkOpen("test_16_rebuild_update.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 5; i++) {
		char data[200];
		sprintf(data, "Original file %d", i);
		xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_16_rebuild_update.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 5; i++) {
		char data[300];
		sprintf(data, "Updated file %d with more content", i);
		ASSERT_EQ(xpkUpdateData(xpk, i, data, (uint32_t)strlen(data), 6), 0);
	}

	ASSERT_EQ(xpkRebuild(xpk), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_16_rebuild_update.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 5);

	for (int i = 0; i < 5; i++) {
		char expected[300];
		sprintf(expected, "Updated file %d with more content", i);

		uint32_t outSize = 0;
		void* data = xpkExtractData(xpk, i, &outSize);
		ASSERT_NOT_NULL(data);
		ASSERT_EQ(memcmp(data, expected, strlen(expected)), 0);
		xpkFree(data);
	}

	xpkClose(xpk);
}

TEST(rebuild_multiple_cycles) {
	char filename[64];
	strcpy(filename, "test_16_rebuild_cycle.xpk");

	for (int cycle = 0; cycle < 3; cycle++) {
		xpkObject xpk = xpkOpen(filename, cycle == 0 ? 0 : 1);
		ASSERT_NOT_NULL(xpk);

		if (cycle == 0) {
			for (int i = 0; i < 8; i++) {
				char data[400];
				sprintf(data, "Cycle 0 file %d", i);
				xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
			}
		}

		ASSERT_EQ(xpkRebuild(xpk), 0);
		ASSERT_EQ(xpkSave(xpk), 0);
		xpkClose(xpk);
	}

	xpkObject xpk = xpkOpen(filename, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 8);

	for (int i = 0; i < 8; i++) {
		char expected[400];
		sprintf(expected, "Cycle 0 file %d", i);

		uint32_t outSize = 0;
		void* data = xpkExtractData(xpk, i, &outSize);
		ASSERT_NOT_NULL(data);
		ASSERT_EQ(memcmp(data, expected, strlen(expected)), 0);
		xpkFree(data);
	}

	xpkClose(xpk);
}

TEST(rebuild_large_files) {
	xpkObject xpk = xpkOpen("test_16_rebuild_large.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t sizes[] = { 5 * 1024 * 1024, 10 * 1024 * 1024, 15 * 1024 * 1024 };
	uint32_t hashes[3];

	for (int i = 0; i < 3; i++) {
		void* data = malloc(sizes[i]);
		ASSERT_NOT_NULL(data);
		memset(data, 'A' + i, sizes[i]);

		hashes[i] = xrtHash32((ptr)data, sizes[i]);

		xpkAppendData(xpk, data, (uint32_t)sizes[i], 6);

		ASSERT_EQ(xpkInfoHash(xpk, i), hashes[i]);

		free(data);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_16_rebuild_large.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkRebuild(xpk), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_16_rebuild_large.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 3);

	for (int i = 0; i < 3; i++) {
		ASSERT_EQ(xpkInfoHash(xpk, i), hashes[i]);

		void* original = malloc(sizes[i]);
		ASSERT_NOT_NULL(original);
		memset(original, 'A' + i, sizes[i]);

		uint32_t outSize = 0;
		void* extracted = xpkExtractData(xpk, i, &outSize);
		ASSERT_NOT_NULL(extracted);
		ASSERT_EQ(outSize, (uint32_t)sizes[i]);
		ASSERT_EQ(memcmp(original, extracted, sizes[i]), 0);

		free(original);
		xpkFree(extracted);
	}

	xpkClose(xpk);
}

TEST(rebuild_path_mode) {
	xpkObject xpk = xpkOpen("test_16_rebuild_path.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	xpkPathAppendData(xpk, "file1.txt", "Content 1", 9, 6);
	xpkPathAppendData(xpk, "dir/file2.txt", "Content 2", 9, 6);
	xpkPathAppendData(xpk, "dir/subdir/file3.txt", "Content 3", 9, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_16_rebuild_path.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkRebuild(xpk), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_16_rebuild_path.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 3);

	ASSERT_EQ(xpkPathExists(xpk, "file1.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "dir/file2.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "dir/subdir/file3.txt"), 1);

	xpkClose(xpk);
}

TEST(rebuild_index_mode) {
	xpkObject xpk = xpkOpen("test_16_rebuild_index.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

	int32_t indices[] = { 10, 20, 30, 40, 50 };
	char data[5][64] = { "Data 10", "Data 20", "Data 30", "Data 40", "Data 50" };
	uint32_t hashes[5];

	for (int i = 0; i < 5; i++) {
		hashes[i] = xrtHash32((ptr)data[i], strlen(data[i]));
		xpkIndexAppendData(xpk, indices[i], data[i], (uint32_t)strlen(data[i]), 6);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_16_rebuild_index.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkRebuild(xpk), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_16_rebuild_index.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 5);

	for (int i = 0; i < 5; i++) {
		ASSERT_EQ(xpkInfoHash(xpk, i), hashes[i]);
		ASSERT_EQ(xpkIndexFind(xpk, indices[i]), i);
	}

	xpkClose(xpk);
}

TEST(rebuild_empty_files) {
	xpkObject xpk = xpkOpen("test_16_rebuild_empty_files.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 10; i++) {
		xpkAppendData(xpk, NULL, 0, 6);
	}

	ASSERT_EQ(xpkCount(xpk), 10);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_16_rebuild_empty_files.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkRebuild(xpk), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_16_rebuild_empty_files.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 10);

	for (int i = 0; i < 10; i++) {
		uint32_t outSize = 0;
		void* data = xpkExtractData(xpk, i, &outSize);
		ASSERT_NOT_NULL(data);
		ASSERT_EQ(outSize, 0);
		xpkFree(data);
	}

	xpkClose(xpk);
}

TEST(rebuild_mixed_levels) {
	xpkObject xpk = xpkOpen("test_16_rebuild_levels.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	uint32_t hashes[8];
	size_t sizes[8];

	for (int i = 0; i < 8; i++) {
		sizes[i] = 1000;
		void* data = malloc(sizes[i]);
		ASSERT_NOT_NULL(data);
		memset(data, 'A' + i, sizes[i]);

		hashes[i] = xrtHash32((ptr)data, sizes[i]);

		int level = i * 2;
		xpkAppendData(xpk, data, (uint32_t)sizes[i], level);

		ASSERT_EQ(xpkInfoLevel(xpk, i), level);
		ASSERT_EQ(xpkInfoHash(xpk, i), hashes[i]);

		free(data);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_16_rebuild_levels.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkRebuild(xpk), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_16_rebuild_levels.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 8);

	for (int i = 0; i < 8; i++) {
		ASSERT_EQ(xpkInfoLevel(xpk, i), i * 2);
		ASSERT_EQ(xpkInfoHash(xpk, i), hashes[i]);

		void* original = malloc(sizes[i]);
		ASSERT_NOT_NULL(original);
		memset(original, 'A' + i, sizes[i]);

		uint32_t outSize = 0;
		void* extracted = xpkExtractData(xpk, i, &outSize);
		ASSERT_NOT_NULL(extracted);
		ASSERT_EQ(outSize, (uint32_t)sizes[i]);
		ASSERT_EQ(memcmp(original, extracted, sizes[i]), 0);

		free(original);
		xpkFree(extracted);
	}

	xpkClose(xpk);
}

int main(void) {
	xrtInit();

	printf("=================================================\n");
	printf("  xPack Ver7 - Rebuild Operations Test\n");
	printf("=================================================\n\n");

	printf("[Basic Rebuild Tests]\n");
	RUN_TEST(rebuild_empty_package);
	RUN_TEST(rebuild_single_file);
	RUN_TEST(rebuild_multiple_files);
	printf("\n");

	printf("[Rebuild After Operations Tests]\n");
	RUN_TEST(rebuild_after_remove);
	RUN_TEST(rebuild_after_update);
	RUN_TEST(rebuild_multiple_cycles);
	printf("\n");

	printf("[Large Files Rebuild Tests]\n");
	RUN_TEST(rebuild_large_files);
	printf("\n");

	printf("[Mode Specific Rebuild Tests]\n");
	RUN_TEST(rebuild_path_mode);
	RUN_TEST(rebuild_index_mode);
	printf("\n");

	printf("[Edge Cases Rebuild Tests]\n");
	RUN_TEST(rebuild_empty_files);
	RUN_TEST(rebuild_mixed_levels);
	printf("\n");

	printf("=================================================\n");
	printf("  Results: %d passed, %d failed\n", tests_passed, tests_failed);
	printf("=================================================\n");

	return tests_failed > 0 ? 1 : 0;
}
