/*
 * xPack Ver7 - 大文件压缩测试
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
#define ASSERT_GT(a, b) ASSERT((a) > (b))
#define ASSERT_LT(a, b) ASSERT((a) < (b))
#define ASSERT_GE(a, b) ASSERT((a) >= (b))

TEST(large_file_10mb) {
	xpkObject xpk = xpkOpen("test_09_10mb.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 10 * 1024 * 1024;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);
	memset(data, 'A', size);

	xpkAppendData(xpk, data, (uint32_t)size, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_09_10mb.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkInfoSize(xpk, 0), (uint32_t)size);
	ASSERT_LT(xpkInfoPacked(xpk, 0), (uint32_t)size);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)size);
	ASSERT_EQ(memcmp(data, extracted, size), 0);

	free(data);
	xpkFree(extracted);
	xpkClose(xpk);
}

TEST(large_file_50mb) {
	xpkObject xpk = xpkOpen("test_09_50mb.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 50 * 1024 * 1024;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);

	for (size_t i = 0; i < size; i++) {
		((uint8_t*)data)[i] = (uint8_t)(i % 256);
	}

	xpkAppendData(xpk, data, (uint32_t)size, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_09_50mb.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkInfoSize(xpk, 0), (uint32_t)size);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)size);
	ASSERT_EQ(memcmp(data, extracted, size), 0);

	free(data);
	xpkFree(extracted);
	xpkClose(xpk);
}

TEST(large_file_100mb) {
	xpkObject xpk = xpkOpen("test_09_100mb.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 100 * 1024 * 1024;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);
	memset(data, 'X', size);

	xpkAppendData(xpk, data, (uint32_t)size, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_09_100mb.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkInfoSize(xpk, 0), (uint32_t)size);
	ASSERT_LT(xpkInfoPacked(xpk, 0), (uint32_t)size);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)size);
	ASSERT_EQ(memcmp(data, extracted, size), 0);

	free(data);
	xpkFree(extracted);
	xpkClose(xpk);
}

TEST(large_file_random_data) {
	xpkObject xpk = xpkOpen("test_09_random_large.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 20 * 1024 * 1024;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);

	srand((unsigned int)time(NULL));
	for (size_t i = 0; i < size; i++) {
		((uint8_t*)data)[i] = (uint8_t)rand();
	}

	uint32_t originalHash = xrtHash32((ptr)data, size);

	xpkAppendData(xpk, data, (uint32_t)size, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_09_random_large.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkInfoHash(xpk, 0), originalHash);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)size);

	uint32_t extractedHash = xrtHash32((ptr)extracted, outSize);
	ASSERT_EQ(extractedHash, originalHash);

	free(data);
	xpkFree(extracted);
	xpkClose(xpk);
}

TEST(multiple_large_files) {
	xpkObject xpk = xpkOpen("test_09_multi_large.xpk", 0, 0);
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

	ASSERT_EQ(xpkCount(xpk), 3);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_09_multi_large.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 3);

	for (int i = 0; i < 3; i++) {
		ASSERT_EQ(xpkInfoHash(xpk, i), hashes[i]);

		uint32_t outSize = 0;
		void* extracted = xpkExtractData(xpk, i, &outSize);
		ASSERT_NOT_NULL(extracted);
		ASSERT_EQ(outSize, (uint32_t)sizes[i]);

		uint32_t extractedHash = xrtHash32((ptr)extracted, outSize);
		ASSERT_EQ(extractedHash, hashes[i]);

		xpkFree(extracted);
	}

	xpkClose(xpk);
}

TEST(large_file_compression_ratio) {
	xpkObject xpk = xpkOpen("test_09_ratio.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 30 * 1024 * 1024;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);
	memset(data, 'Z', size);

	xpkAppendData(xpk, data, (uint32_t)size, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_09_ratio.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t packedSize = xpkInfoPacked(xpk, 0);

	float ratio = (float)packedSize / size;
	ASSERT_LT(ratio, 0.02f);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)size);
	ASSERT_EQ(memcmp(data, extracted, size), 0);

	free(data);
	xpkFree(extracted);
	xpkClose(xpk);
}

TEST(large_file_high_compression) {
	xpkObject xpk = xpkOpen("test_09_high_comp.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 25 * 1024 * 1024;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);

	for (size_t i = 0; i < size; i++) {
		((uint8_t*)data)[i] = (uint8_t)(i % 16);
	}

	xpkAppendData(xpk, data, (uint32_t)size, 15);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_09_high_comp.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkInfoLevel(xpk, 0), 15);
	ASSERT_EQ(xpkInfoSize(xpk, 0), (uint32_t)size);

	uint32_t packedSize = xpkInfoPacked(xpk, 0);
	ASSERT_GT(packedSize, 0);
	ASSERT_LT(packedSize, (uint32_t)size);

	float ratio = (float)packedSize / size;
	ASSERT_LT(ratio, 0.05f);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)size);
	ASSERT_EQ(memcmp(data, extracted, size), 0);

	free(data);
	xpkFree(extracted);
	xpkClose(xpk);
}

TEST(large_file_no_compression) {
	xpkObject xpk = xpkOpen("test_09_no_comp.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 15 * 1024 * 1024;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);

	srand((unsigned int)time(NULL));
	for (size_t i = 0; i < size; i++) {
		((uint8_t*)data)[i] = (uint8_t)rand();
	}

	xpkAppendData(xpk, data, (uint32_t)size, 0);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_09_no_comp.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkInfoLevel(xpk, 0), 0);
	ASSERT_EQ(xpkInfoSize(xpk, 0), (uint32_t)size);
	ASSERT_EQ(xpkInfoPacked(xpk, 0), (uint32_t)size);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)size);
	ASSERT_EQ(memcmp(data, extracted, size), 0);

	free(data);
	xpkFree(extracted);
	xpkClose(xpk);
}

TEST(large_file_update) {
	xpkObject xpk = xpkOpen("test_09_large_update.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size1 = 20 * 1024 * 1024;
	void* data1 = malloc(size1);
	ASSERT_NOT_NULL(data1);
	memset(data1, 'A', size1);

	xpkAppendData(xpk, data1, (uint32_t)size1, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_09_large_update.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size2 = 25 * 1024 * 1024;
	void* data2 = malloc(size2);
	ASSERT_NOT_NULL(data2);
	memset(data2, 'B', size2);

	ASSERT_EQ(xpkUpdateData(xpk, 0, data2, (uint32_t)size2, 6), 0);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_09_large_update.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkInfoSize(xpk, 0), (uint32_t)size2);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)size2);
	ASSERT_EQ(memcmp(data2, extracted, size2), 0);

	free(data1);
	free(data2);
	xpkFree(extracted);
	xpkClose(xpk);
}

TEST(large_file_rebuild) {
	xpkObject xpk = xpkOpen("test_09_large_rebuild.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	uint32_t hashes[5];
	size_t sizes[5];

	for (int i = 0; i < 5; i++) {
		sizes[i] = (i + 1) * 5 * 1024 * 1024;
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

	xpk = xpkOpen("test_09_large_rebuild.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkRebuild(xpk), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_09_large_rebuild.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 5);

	for (int i = 0; i < 5; i++) {
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

TEST(large_file_path_mode) {
	xpkObject xpk = xpkOpen("test_09_large_path.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	size_t size = 12 * 1024 * 1024;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);
	memset(data, 'P', size);

	uint32_t originalHash = xrtHash32((ptr)data, size);

	xpkPathAppendData(xpk, "large/file.dat", data, (uint32_t)size, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_09_large_path.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkPathExists(xpk, "large/file.dat"), 1);
	ASSERT_EQ(xpkInfoHash(xpk, 0), originalHash);

	uint32_t outSize = 0;
	void* extracted = xpkPathExtractData(xpk, "large/file.dat", &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)size);
	ASSERT_EQ(memcmp(data, extracted, size), 0);

	free(data);
	xpkFree(extracted);
	xpkClose(xpk);
}

int main(void) {
	xrtInit();

	printf("=================================================\n");
	printf("  xPack Ver7 - Large Files Compression Test\n");
	printf("=================================================\n\n");

	printf("[Large File Size Tests]\n");
	RUN_TEST(large_file_10mb);
	RUN_TEST(large_file_50mb);
	RUN_TEST(large_file_100mb);
	printf("\n");

	printf("[Large File Data Pattern Tests]\n");
	RUN_TEST(large_file_random_data);
	RUN_TEST(large_file_compression_ratio);
	printf("\n");

	printf("[Multiple Large Files Tests]\n");
	RUN_TEST(multiple_large_files);
	printf("\n");

	printf("[Large File Compression Level Tests]\n");
	RUN_TEST(large_file_high_compression);
	RUN_TEST(large_file_no_compression);
	printf("\n");

	printf("[Large File Operation Tests]\n");
	RUN_TEST(large_file_update);
	RUN_TEST(large_file_rebuild);
	RUN_TEST(large_file_path_mode);
	printf("\n");

	printf("=================================================\n");
	printf("  Results: %d passed, %d failed\n", tests_passed, tests_failed);
	printf("=================================================\n");

	return tests_failed > 0 ? 1 : 0;
}
