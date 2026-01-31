/*
 * xPack Ver7 - compression_data_patterns (07)
 */

#include "test_framework.h"

TEST(pattern_all_zeros) {
	xpkObject xpk = xpkOpen("test_07_zeros.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 10000;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);
	memset(data, 0, size);

	xpkAppendData(xpk, data, (uint32_t)size, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_07_zeros.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t packedSize = xpkInfoPacked(xpk, 0);
	ASSERT_GT(packedSize, 0);
	ASSERT_LT(packedSize, (uint32_t)size);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)size);
	ASSERT_EQ(memcmp(data, extracted, size), 0);

	free(data);
	xpkFree(extracted);
	xpkClose(xpk);
}

TEST(pattern_all_ones) {
	xpkObject xpk = xpkOpen("test_07_ones.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 10000;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);
	memset(data, 0xFF, size);

	xpkAppendData(xpk, data, (uint32_t)size, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_07_ones.xpk", 0, 1);
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

TEST(pattern_repeated_byte) {
	xpkObject xpk = xpkOpen("test_07_repeat.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 5000;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);
	memset(data, 'A', size);

	xpkAppendData(xpk, data, (uint32_t)size, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_07_repeat.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t packedSize = xpkInfoPacked(xpk, 0);
	ASSERT_GT(packedSize, 0);
	ASSERT_LT(packedSize, (uint32_t)size);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)size);
	ASSERT_EQ(memcmp(data, extracted, size), 0);

	free(data);
	xpkFree(extracted);
	xpkClose(xpk);
}

TEST(pattern_sequential_bytes) {
	xpkObject xpk = xpkOpen("test_07_sequential.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 8000;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);
	for (size_t i = 0; i < size; i++) {
		((uint8_t*)data)[i] = (uint8_t)i;
	}

	xpkAppendData(xpk, data, (uint32_t)size, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_07_sequential.xpk", 0, 1);
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

TEST(pattern_alternating) {
	xpkObject xpk = xpkOpen("test_07_alternating.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 6000;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);
	for (size_t i = 0; i < size; i++) {
		((uint8_t*)data)[i] = (uint8_t)(i % 2);
	}

	xpkAppendData(xpk, data, (uint32_t)size, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_07_alternating.xpk", 0, 1);
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

TEST(pattern_random) {
	xpkObject xpk = xpkOpen("test_07_random.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 7000;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);
	srand((unsigned int)time(NULL));
	for (size_t i = 0; i < size; i++) {
		((uint8_t*)data)[i] = (uint8_t)rand();
	}

	xpkAppendData(xpk, data, (uint32_t)size, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_07_random.xpk", 0, 1);
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

TEST(pattern_text_data) {
	xpkObject xpk = xpkOpen("test_07_text.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 4000;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);
	for (size_t i = 0; i < size; i++) {
		((char*)data)[i] = 'a' + (char)(i % 26);
	}

	xpkAppendData(xpk, data, (uint32_t)size, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_07_text.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t packedSize = xpkInfoPacked(xpk, 0);
	ASSERT_GT(packedSize, 0);
	ASSERT_LT(packedSize, (uint32_t)size);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)size);
	ASSERT_EQ(memcmp(data, extracted, size), 0);

	free(data);
	xpkFree(extracted);
	xpkClose(xpk);
}

TEST(pattern_binary_data) {
	xpkObject xpk = xpkOpen("test_07_binary.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 5000;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);
	srand((unsigned int)time(NULL));
	for (size_t i = 0; i < size; i++) {
		((uint8_t*)data)[i] = (uint8_t)rand();
	}

	xpkAppendData(xpk, data, (uint32_t)size, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_07_binary.xpk", 0, 1);
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

TEST(pattern_repeated_block) {
	xpkObject xpk = xpkOpen("test_07_block.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t blockSize = 100;
	size_t size = blockSize * 50;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);

	for (size_t i = 0; i < blockSize; i++) {
		((uint8_t*)data)[i] = (uint8_t)i;
	}

	for (size_t i = blockSize; i < size; i++) {
		((uint8_t*)data)[i] = ((uint8_t*)data)[i % blockSize];
	}

	xpkAppendData(xpk, data, (uint32_t)size, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_07_block.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t packedSize = xpkInfoPacked(xpk, 0);
	ASSERT_GT(packedSize, 0);
	ASSERT_LT(packedSize, (uint32_t)size);

	uint32_t outSize = 0;
	void* extracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(extracted);
	ASSERT_EQ(outSize, (uint32_t)size);
	ASSERT_EQ(memcmp(data, extracted, size), 0);

	free(data);
	xpkFree(extracted);
	xpkClose(xpk);
}

TEST(pattern_mixed) {
	xpkObject xpk = xpkOpen("test_07_mixed.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	size_t size = 9000;
	void* data = malloc(size);
	ASSERT_NOT_NULL(data);

	memset(data, 0, size / 4);
	for (size_t i = size / 4; i < size / 2; i++) {
		((uint8_t*)data)[i] = (uint8_t)(i * 2);
	}
	for (size_t i = size / 2; i < size * 3 / 4; i++) {
		((uint8_t*)data)[i] = (uint8_t)rand();
	}
	memset((uint8_t*)data + size * 3 / 4, 0xFF, size / 4);

	xpkAppendData(xpk, data, (uint32_t)size, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_07_mixed.xpk", 0, 1);
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

TEST(pattern_all_algorithms) {
	int levels[] = {1, 6, 11};
	const char* names[] = {"LZ4", "ZSTD", "LZMA2"};

	for (int i = 0; i < 3; i++) {
		char filename[64];
		sprintf(filename, "test_07_algo_%s.xpk", names[i]);

		xpkObject xpk = xpkOpen(filename, 0, 0);
		ASSERT_NOT_NULL(xpk);

		size_t size = 3000;
		void* data = malloc(size);
		ASSERT_NOT_NULL(data);
		memset(data, 'X', size);

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

TEST(pattern_compression_ratios) {
	struct {
		const char* name;
		void* data;
		size_t size;
	} patterns[] = {
		{"zeros", NULL, 10000},
		{"ones", NULL, 10000},
		{"repeat", NULL, 5000},
		{"random", NULL, 5000},
	};

	for (int i = 0; i < 4; i++) {
		char filename[64];
		sprintf(filename, "test_07_ratio_%s.xpk", patterns[i].name);

		patterns[i].data = malloc(patterns[i].size);
		ASSERT_NOT_NULL(patterns[i].data);

		if (strcmp(patterns[i].name, "zeros") == 0) {
			memset(patterns[i].data, 0, patterns[i].size);
		} else if (strcmp(patterns[i].name, "ones") == 0) {
			memset(patterns[i].data, 0xFF, patterns[i].size);
		} else if (strcmp(patterns[i].name, "repeat") == 0) {
			memset(patterns[i].data, 'A', patterns[i].size);
		} else {
			srand((unsigned int)time(NULL));
			for (size_t j = 0; j < patterns[i].size; j++) {
				((uint8_t*)patterns[i].data)[j] = (uint8_t)rand();
			}
		}

		xpkObject xpk = xpkOpen(filename, 0, 0);
		ASSERT_NOT_NULL(xpk);

		xpkAppendData(xpk, patterns[i].data, (uint32_t)patterns[i].size, 6);
		ASSERT_EQ(xpkSave(xpk), 0);
		xpkClose(xpk);

		xpk = xpkOpen(filename, 0, 1);
		ASSERT_NOT_NULL(xpk);

		uint32_t packedSize = xpkInfoPacked(xpk, 0);
		ASSERT_GT(packedSize, 0);

		free(patterns[i].data);
		xpkClose(xpk);
	}
}

void register_07_compression_data_patterns_tests(void) {
	TEST_REGISTER(pattern_all_zeros, CAT_COMPRESSION, "Test all zeros pattern");
	TEST_REGISTER(pattern_all_ones, CAT_COMPRESSION, "Test all ones pattern");
	TEST_REGISTER(pattern_repeated_byte, CAT_COMPRESSION, "Test repeated byte pattern");
	TEST_REGISTER(pattern_sequential_bytes, CAT_COMPRESSION, "Test sequential bytes pattern");
	TEST_REGISTER(pattern_alternating, CAT_COMPRESSION, "Test alternating pattern");
	TEST_REGISTER(pattern_random, CAT_COMPRESSION, "Test random pattern");
	TEST_REGISTER(pattern_text_data, CAT_COMPRESSION, "Test text data pattern");
	TEST_REGISTER(pattern_binary_data, CAT_COMPRESSION, "Test binary data pattern");
	TEST_REGISTER(pattern_repeated_block, CAT_COMPRESSION, "Test repeated block pattern");
	TEST_REGISTER(pattern_mixed, CAT_COMPRESSION, "Test mixed pattern");
	TEST_REGISTER(pattern_all_algorithms, CAT_COMPRESSION, "Test all algorithms");
	TEST_REGISTER(pattern_compression_ratios, CAT_COMPRESSION, "Test compression ratios");
}
