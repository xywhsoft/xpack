/*
 * xPack Ver7 - 校验功能测试
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
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

TEST(verify_single_file) {
	xpkObject xpk = xpkOpen("test_15_verify_single.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	char data[1000];
	memset(data, 'A', sizeof(data));

	xpkAppendData(xpk, data, sizeof(data), 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_15_verify_single.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkVerify(xpk, 0), 0);

	xpkClose(xpk);
}

TEST(verify_all_files) {
	xpkObject xpk = xpkOpen("test_15_verify_all.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 10; i++) {
		char data[500];
		memset(data, 'A' + i, sizeof(data));
		xpkAppendData(xpk, data, sizeof(data), 6);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_15_verify_all.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	xpkClose(xpk);
}

TEST(verify_empty_package) {
	xpkObject xpk = xpkOpen("test_15_verify_empty.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_15_verify_empty.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	xpkClose(xpk);
}

TEST(verify_large_files) {
	xpkObject xpk = xpkOpen("test_15_verify_large.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t sizes[] = { 1024 * 1024, 2 * 1024 * 1024, 5 * 1024 * 1024 };

	for (int i = 0; i < 3; i++) {
		void* data = malloc(sizes[i]);
		ASSERT_NOT_NULL(data);
		memset(data, 'A' + i, sizes[i]);
		xpkAppendData(xpk, data, (uint32_t)sizes[i], 6);
		free(data);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_15_verify_large.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	xpkClose(xpk);
}

TEST(verify_different_levels) {
	char filename[64];
	strcpy(filename, "test_15_verify_levels.xpk");

	xpkObject xpk = xpkOpen(filename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int level = 0; level <= 15; level++) {
		char data[2000];
		memset(data, 'X', sizeof(data));
		xpkAppendData(xpk, data, sizeof(data), level);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(filename, 0, 1);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i <= 15; i++) {
		ASSERT_EQ(xpkVerify(xpk, i), 0);
	}

	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	xpkClose(xpk);
}

TEST(verify_after_update) {
	char filename[64];
	strcpy(filename, "test_15_verify_update.xpk");

	xpkObject xpk = xpkOpen(filename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 5; i++) {
		char data[800];
		sprintf(data, "Original data %d", i);
		xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(filename, 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	for (int i = 0; i < 5; i++) {
		char newData[1000];
		sprintf(newData, "Updated data %d with more content", i);
		ASSERT_EQ(xpkUpdateData(xpk, i, newData, (uint32_t)strlen(newData), 6), 0);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(filename, 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	xpkClose(xpk);
}

TEST(verify_after_rebuild) {
	char filename[64];
	strcpy(filename, "test_15_verify_rebuild.xpk");

	xpkObject xpk = xpkOpen(filename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 8; i++) {
		char data[1000];
		memset(data, 'A' + i, sizeof(data));
		xpkAppendData(xpk, data, sizeof(data), 6);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(filename, 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkRebuild(xpk), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(filename, 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	xpkClose(xpk);
}

TEST(verify_path_mode) {
	xpkObject xpk = xpkOpen("test_15_verify_path.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	xpkPathAppendData(xpk, "file1.txt", "Content 1", 9, 6);
	xpkPathAppendData(xpk, "dir/file2.txt", "Content 2", 9, 6);
	xpkPathAppendData(xpk, "dir/subdir/file3.txt", "Content 3", 9, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_15_verify_path.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	ASSERT_EQ(xpkVerify(xpk, 0), 0);
	ASSERT_EQ(xpkVerify(xpk, 1), 0);
	ASSERT_EQ(xpkVerify(xpk, 2), 0);

	xpkClose(xpk);
}

TEST(verify_index_mode) {
	xpkObject xpk = xpkOpen("test_15_verify_index.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

	for (int i = 0; i < 5; i++) {
		char data[600];
		sprintf(data, "Index data %d", i);
		xpkIndexAppendData(xpk, i * 10, data, (uint32_t)strlen(data), 6);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_15_verify_index.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	for (int i = 0; i < 5; i++) {
		ASSERT_EQ(xpkVerify(xpk, i), 0);
	}

	xpkClose(xpk);
}

TEST(verify_hash_mismatch) {
	xpkObject xpk = xpkOpen("test_15_verify_mismatch.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	char data[1000];
	memset(data, 'A', sizeof(data));

	xpkAppendData(xpk, data, sizeof(data), 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	FILE* fp = fopen("test_15_verify_mismatch.xpk", "r+b");
	ASSERT_NOT_NULL(fp);

	fseek(fp, 0, SEEK_END);
	long fileSize = ftell(fp);

	char ldbHash[4] = { 0, 0, 0, 0 };
	fseek(fp, fileSize - 8, SEEK_SET);
	fwrite(ldbHash, 1, 4, fp);
	fclose(fp);

	xpk = xpkOpen("test_15_verify_mismatch.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_NE(xpkVerifyAll(xpk), 0);

	xpkClose(xpk);
}

TEST(verify_mixed_compression) {
	xpkObject xpk = xpkOpen("test_15_verify_mixed.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int level = 0; level <= 15; level += 3) {
		char data[1500];
		memset(data, 'A' + level, sizeof(data));
		xpkAppendData(xpk, data, sizeof(data), level);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_15_verify_mixed.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	xpkClose(xpk);
}

TEST(verify_empty_files) {
	xpkObject xpk = xpkOpen("test_15_verify_empty_files.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 5; i++) {
		xpkAppendData(xpk, NULL, 0, 6);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_15_verify_empty_files.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	for (int i = 0; i < 5; i++) {
		ASSERT_EQ(xpkVerify(xpk, i), 0);
	}

	xpkClose(xpk);
}

TEST(verify_single_byte_files) {
	xpkObject xpk = xpkOpen("test_15_verify_single_byte.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 10; i++) {
		uint8_t byte = (uint8_t)i;
		xpkAppendData(xpk, &byte, 1, 6);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_15_verify_single_byte.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	for (int i = 0; i < 10; i++) {
		ASSERT_EQ(xpkVerify(xpk, i), 0);
	}

	xpkClose(xpk);
}

TEST(verify_random_data) {
	xpkObject xpk = xpkOpen("test_15_verify_random.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 5; i++) {
		size_t size = (i + 1) * 2000;
		void* data = malloc(size);
		ASSERT_NOT_NULL(data);

		srand((unsigned int)(time(NULL) + i));
		for (size_t j = 0; j < size; j++) {
			((uint8_t*)data)[j] = (uint8_t)rand();
		}

		xpkAppendData(xpk, data, (uint32_t)size, 6);
		free(data);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_15_verify_random.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkVerifyAll(xpk), 0);

	for (int i = 0; i < 5; i++) {
		ASSERT_EQ(xpkVerify(xpk, i), 0);
	}

	xpkClose(xpk);
}

int main(void) {
	xrtInit();

	printf("=================================================\n");
	printf("  xPack Ver7 - Verify Operations Test\n");
	printf("=================================================\n\n");

	printf("[Basic Verify Tests]\n");
	RUN_TEST(verify_single_file);
	RUN_TEST(verify_all_files);
	RUN_TEST(verify_empty_package);
	printf("\n");

	printf("[Large Files Verify Tests]\n");
	RUN_TEST(verify_large_files);
	printf("\n");

	printf("[Compression Level Verify Tests]\n");
	RUN_TEST(verify_different_levels);
	RUN_TEST(verify_mixed_compression);
	printf("\n");

	printf("[Verify After Operations Tests]\n");
	RUN_TEST(verify_after_update);
	RUN_TEST(verify_after_rebuild);
	printf("\n");

	printf("[Mode Specific Verify Tests]\n");
	RUN_TEST(verify_path_mode);
	RUN_TEST(verify_index_mode);
	printf("\n");

	printf("[Edge Cases Verify Tests]\n");
	RUN_TEST(verify_hash_mismatch);
	RUN_TEST(verify_empty_files);
	RUN_TEST(verify_single_byte_files);
	RUN_TEST(verify_random_data);
	printf("\n");

	printf("=================================================\n");
	printf("  Results: %d passed, %d failed\n", tests_passed, tests_failed);
	printf("=================================================\n");

	return tests_failed > 0 ? 1 : 0;
}
