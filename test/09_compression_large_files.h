/*
 * xPack Ver7 - compression_large_files (09)
 */

#include "test_framework.h"

TEST(large_file_1mb) {
	xpkObject xpk = xpkOpen("test_09_1mb.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 1024 * 1024;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);
	memset(data, 'A', size);

	xpkAppendData(xpk, data, (uint32_t)size, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_09_1mb.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)size);
	ASSERT_EQ(memcmp(data, extracted, size), 0);

	free(data);
	xpkFree(extracted);
	xpkClose(xpk);
}

TEST(large_file_10mb) {
	xpkObject xpk = xpkOpen("test_09_10mb.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 10 * 1024 * 1024;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);
	memset(data, 'B', size);

	xpkAppendData(xpk, data, (uint32_t)size, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_09_10mb.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)size);
	ASSERT_EQ(memcmp(data, extracted, size), 0);

	free(data);
	xpkFree(extracted);
	xpkClose(xpk);
}

TEST(large_file_compression_levels) {
	for (int level = 0; level <= 15; level++) {
		char filename[64];
		sprintf(filename, "test_09_level_%d.xpk", level);

		xpkObject xpk = xpkOpen(filename, 0, 0);
		ASSERT_NOT_NULL(xpk);

		size_t size = 2 * 1024 * 1024;
		void* data = malloc(size);
		ASSERT_NOT_NULL(data);
		memset(data, 'C', size);

		xpkAppendData(xpk, data, (uint32_t)size, level);
		ASSERT_EQ(xpkSave(xpk), 0);
		xpkClose(xpk);

		xpk = xpkOpen(filename, 0, 1);
		ASSERT_NOT_NULL(xpk);

		uint32_t outSize = 0;
		void* extracted = xpkExtractData(xpk, 0, &outSize);
		ASSERT_NOT_NULL(extracted);
		ASSERT_EQ(outSize, (uint32_t)size);
		ASSERT_EQ(memcmp(data, extracted, size), 0);

		free(data);
		xpkFree(extracted);
		xpkClose(xpk);
	}
}

TEST(large_file_multiple) {
	xpkObject xpk = xpkOpen("test_09_multiple.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 5; i++) {
		size_t size = 1024 * 1024;
		void* data = malloc(size);
		ASSERT_NOT_NULL(data);
		memset(data, 'A' + i, size);

		xpkAppendData(xpk, data, (uint32_t)size, 6);
		free(data);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 5);
	xpkClose(xpk);

	xpk = xpkOpen("test_09_multiple.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 5);

	for (uint32_t i = 0; i < 5; i++) {
		uint32_t outSize = 0;
		void* extracted = xpkExtractData(xpk, i, &outSize);
		ASSERT_NOT_NULL(extracted);
		ASSERT_EQ(outSize, 1024 * 1024);
		xpkFree(extracted);
	}

	xpkClose(xpk);
}

TEST(large_file_random_data) {
	xpkObject xpk = xpkOpen("test_09_random.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 5 * 1024 * 1024;
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

	xpk = xpkOpen("test_09_random.xpk", 0, 1);
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

TEST(large_file_sequential_data) {
	xpkObject xpk = xpkOpen("test_09_sequential.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 3 * 1024 * 1024;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);

	for (size_t i = 0; i < size; i++) {
		((uint8_t*)data)[i] = (uint8_t)i;
	}

	xpkAppendData(xpk, data, (uint32_t)size, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_09_sequential.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

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
	xpkObject xpk = xpkOpen("test_09_update.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 2 * 1024 * 1024;
	void* data1 = malloc(size);
	ASSERT_NOT_NULL(data1);
	memset(data1, 'X', size);

	xpkAppendData(xpk, data1, (uint32_t)size, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	void* data2 = malloc(size);
	ASSERT_NOT_NULL(data2);
	memset(data2, 'Y', size);

	xpk = xpkOpen("test_09_update.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkUpdateData(xpk, 0, data2, (uint32_t)size, 6), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_09_update.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)size);
	ASSERT_EQ(memcmp(data2, extracted, size), 0);

	free(data1);
	free(data2);
	xpkFree(extracted);
	xpkClose(xpk);
}

TEST(large_file_extract_partial) {
	xpkObject xpk = xpkOpen("test_09_partial.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 2 * 1024 * 1024;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);
	memset(data, 'Z', size);

	xpkAppendData(xpk, data, (uint32_t)size, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_09_partial.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)size);
	ASSERT_EQ(memcmp(data, extracted, size), 0);

	free(data);
	xpkFree(extracted);
	xpkClose(xpk);
}

TEST(large_file_all_algorithms) {
	int levels[] = {1, 6, 11};
	const char* names[] = {"LZ4", "ZSTD", "LZMA2"};

	for (int i = 0; i < 3; i++) {
		char filename[64];
		sprintf(filename, "test_09_algo_%s.xpk", names[i]);

		xpkObject xpk = xpkOpen(filename, 0, 0);
		ASSERT_NOT_NULL(xpk);

		size_t size = 2 * 1024 * 1024;
		void* data = malloc(size);
		ASSERT_NOT_NULL(data);
		memset(data, 'A' + i, size);

		xpkAppendData(xpk, data, (uint32_t)size, levels[i]);
		ASSERT_EQ(xpkSave(xpk), 0);
		xpkClose(xpk);

		xpk = xpkOpen(filename, 0, 1);
		ASSERT_NOT_NULL(xpk);

		uint32_t outSize = 0;
		void* extracted = xpkExtractData(xpk, 0, &outSize);
		ASSERT_NOT_NULL(extracted);
		ASSERT_EQ(outSize, (uint32_t)size);
		ASSERT_EQ(memcmp(data, extracted, size), 0);

		free(data);
		xpkFree(extracted);
		xpkClose(xpk);
	}
}

TEST(large_file_compression_ratio) {
	xpkObject xpk = xpkOpen("test_09_ratio.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 5 * 1024 * 1024;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);
	memset(data, 'A', size);

	xpkAppendData(xpk, data, (uint32_t)size, 9);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_09_ratio.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t packedSize = xpkInfoPacked(xpk, 0);
	ASSERT_GT(packedSize, 0);
	ASSERT_LT(packedSize, (uint32_t)size);

	double ratio = (double)packedSize / size;
	ASSERT_LT(ratio, 0.5);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)size);
	ASSERT_EQ(memcmp(data, extracted, size), 0);

	free(data);
	xpkFree(extracted);
	xpkClose(xpk);
}

void register_09_compression_large_files_tests(void) {
	TEST_REGISTER(large_file_1mb, CAT_COMPRESSION, "Test 1MB large file");
	TEST_REGISTER(large_file_10mb, CAT_COMPRESSION, "Test 10MB large file");
	TEST_REGISTER(large_file_compression_levels, CAT_COMPRESSION, "Test large file with all compression levels");
	TEST_REGISTER(large_file_multiple, CAT_COMPRESSION, "Test multiple large files");
	TEST_REGISTER(large_file_random_data, CAT_COMPRESSION, "Test large file with random data");
	TEST_REGISTER(large_file_sequential_data, CAT_COMPRESSION, "Test large file with sequential data");
	TEST_REGISTER(large_file_update, CAT_COMPRESSION, "Test large file update");
	TEST_REGISTER(large_file_extract_partial, CAT_COMPRESSION, "Test large file extract partial");
	TEST_REGISTER(large_file_all_algorithms, CAT_COMPRESSION, "Test large file with all algorithms");
	TEST_REGISTER(large_file_compression_ratio, CAT_COMPRESSION, "Test large file compression ratio");
}
