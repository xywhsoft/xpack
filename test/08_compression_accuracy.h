/*
 * xPack Ver7 - compression_accuracy (08)
 */

#include "test_framework.h"

TEST(accuracy_all_levels_byte_for_byte) {
	for (int level = 0; level <= 15; level++) {
		char filename[64];
		sprintf(filename, "test_08_accuracy_level_%d.xpk", level);

		xpkObject xpk = xpkOpen(filename, 0, 0);
		ASSERT_NOT_NULL(xpk);

		size_t size = 10000;
		void* data = malloc(size);
		ASSERT_NOT_NULL(data);

		srand((unsigned int)time(NULL));
		for (size_t i = 0; i < size; i++) {
			((uint8_t*)data)[i] = (uint8_t)rand();
		}

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

TEST(accuracy_hash_consistency) {
	for (int level = 0; level <= 15; level++) {
		char filename[64];
		sprintf(filename, "test_08_hash_level_%d.xpk", level);

		xpkObject xpk = xpkOpen(filename, 0, 0);
		ASSERT_NOT_NULL(xpk);

		size_t size = 5000;
		void* data = malloc(size);
		ASSERT_NOT_NULL(data);
		memset(data, 'X', size);

		uint32_t originalHash = xrtHash32((ptr)data, size);

		xpkAppendData(xpk, data, (uint32_t)size, level);
		ASSERT_EQ(xpkSave(xpk), 0);
		xpkClose(xpk);

		xpk = xpkOpen(filename, 0, 1);
		ASSERT_NOT_NULL(xpk);

		ASSERT_EQ(xpkInfoLevel(xpk, 0), level);
		ASSERT_EQ(xpkInfoHash(xpk, 0), originalHash);

		uint32_t outSize = 0;
		void* extracted = xpkExtractData(xpk, 0, &outSize);
		ASSERT_NOT_NULL(extracted);

		uint32_t extractedHash = xrtHash32((ptr)extracted, outSize);
		ASSERT_EQ(extractedHash, originalHash);
		ASSERT_EQ(extractedHash, xpkInfoHash(xpk, 0));

		free(data);
		xpkFree(extracted);
		xpkClose(xpk);
	}
}

TEST(accuracy_multiple_files_independent) {
	char filename[64];
	strcpy(filename, "test_08_multi_accuracy.xpk");

	for (int testRun = 0; testRun < 3; testRun++) {
		xpkObject xpk = xpkOpen(filename, 0, 0);
		ASSERT_NOT_NULL(xpk);

		for (int i = 0; i < 5; i++) {
			size_t size = (i + 1) * 1000;
			void* data = malloc(size);
			ASSERT_NOT_NULL(data);
			memset(data, 'A' + i, size);

			uint32_t originalHash = xrtHash32((ptr)data, size);

			xpkAppendData(xpk, data, (uint32_t)size, 6);

			ASSERT_EQ(xpkInfoHash(xpk, i), originalHash);

			free(data);
		}

		ASSERT_EQ(xpkSave(xpk), 0);
		xpkClose(xpk);

		xpk = xpkOpen(filename, 0, 1);
		ASSERT_NOT_NULL(xpk);
		ASSERT_EQ(xpkCount(xpk), 5);

		for (int i = 0; i < 5; i++) {
			size_t size = (i + 1) * 1000;
			void* original = malloc(size);
			ASSERT_NOT_NULL(original);
			memset(original, 'A' + i, size);

			uint32_t originalHash = xrtHash32((ptr)original, size);

			uint32_t outSize = 0;
			void* extracted = xpkExtractData(xpk, i, &outSize);
			ASSERT_NOT_NULL(extracted);
			ASSERT_EQ(outSize, (uint32_t)size);

			ASSERT_EQ(memcmp(original, extracted, size), 0);

			uint32_t extractedHash = xrtHash32((ptr)extracted, outSize);
			ASSERT_EQ(extractedHash, originalHash);
			ASSERT_EQ(extractedHash, xpkInfoHash(xpk, i));

			free(original);
			xpkFree(extracted);
		}

		xpkClose(xpk);
	}
}

TEST(accuracy_compress_decompress_cycle) {
	for (int cycle = 0; cycle < 5; cycle++) {
		char filename[64];
		sprintf(filename, "test_08_cycle_%d.xpk", cycle);

		xpkObject xpk = xpkOpen(filename, 0, 0);
		ASSERT_NOT_NULL(xpk);

		size_t size = 8000;
		void* original = malloc(size);
		ASSERT_NOT_NULL(original);

		srand((unsigned int)time(NULL));
		for (size_t i = 0; i < size; i++) {
			((uint8_t*)original)[i] = (uint8_t)rand();
		}

		uint32_t originalHash = xrtHash32((ptr)original, size);

		xpkAppendData(xpk, original, (uint32_t)size, 6);
		ASSERT_EQ(xpkSave(xpk), 0);
		xpkClose(xpk);

		xpk = xpkOpen(filename, 0, 1);
		ASSERT_NOT_NULL(xpk);

		uint32_t outSize = 0;
		void* extracted = xpkExtractData(xpk, 0, &outSize);
		ASSERT_NOT_NULL(extracted);
		ASSERT_EQ(outSize, (uint32_t)size);

		ASSERT_EQ(memcmp(original, extracted, size), 0);

		uint32_t extractedHash = xrtHash32((ptr)extracted, outSize);
		ASSERT_EQ(extractedHash, originalHash);

		free(original);
		xpkFree(extracted);
		xpkClose(xpk);
	}
}

TEST(accuracy_file_operations) {
	char filename[64];
	strcpy(filename, "test_08_file_accuracy.xpk");

	for (int i = 0; i < 5; i++) {
		char srcPath[256];
		sprintf(srcPath, "test_08_src_%d.bin", i);

		FILE* fp = fopen(srcPath, "wb");
		ASSERT_NOT_NULL(fp);

		size_t size = (i + 1) * 2000;
		void* data = malloc(size);
		ASSERT_NOT_NULL(data);

		srand((unsigned int)(time(NULL) + i));
		for (size_t j = 0; j < size; j++) {
			((uint8_t*)data)[j] = (uint8_t)rand();
		}

		fwrite(data, 1, size, fp);
		fclose(fp);

		uint32_t originalHash = xrtHash32((ptr)data, size);

		free(data);

		xpkObject xpk = xpkOpen(filename, 0, i == 0 ? 0 : 1);
		ASSERT_NOT_NULL(xpk);

		if (i == 0) {
			ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);
		}

		char packPath[256];
		sprintf(packPath, "files/file%d.bin", i);
		xpkPathAppendFile(xpk, packPath, srcPath, 6);

		ASSERT_EQ(xpkInfoHash(xpk, i), originalHash);

		ASSERT_EQ(xpkSave(xpk), 0);
		xpkClose(xpk);

		xpk = xpkOpen(filename, 0, 1);
		ASSERT_NOT_NULL(xpk);

		char dstPath[256];
		sprintf(dstPath, "test_08_dst_%d.bin", i);
		ASSERT_EQ(xpkPathExtractFile(xpk, packPath, dstPath), 0);

		fp = fopen(dstPath, "rb");
		ASSERT_NOT_NULL(fp);

		void* extracted = malloc(size);
		ASSERT_NOT_NULL(extracted);
		size_t readSize = fread(extracted, 1, size, fp);
		fclose(fp);

		ASSERT_EQ(readSize, size);
		ASSERT_EQ(memcmp(data, extracted, size), 0);

		uint32_t extractedHash = xrtHash32((ptr)extracted, readSize);
		ASSERT_EQ(extractedHash, originalHash);

		free(extracted);
		xpkClose(xpk);

		xrtFileDelete(srcPath);
		xrtFileDelete(dstPath);
	}
}

TEST(accuracy_update_preserves_data) {
	char filename[64];
	strcpy(filename, "test_08_update_accuracy.xpk");

	xpkObject xpk = xpkOpen(filename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 3; i++) {
		size_t size = 3000;
		void* data = malloc(size);
		ASSERT_NOT_NULL(data);
		memset(data, 'A' + i, size);

		uint32_t originalHash = xrtHash32((ptr)data, size);

		xpkAppendData(xpk, data, (uint32_t)size, 6);

		ASSERT_EQ(xpkInfoHash(xpk, i), originalHash);

		free(data);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);
}

TEST(accuracy_remove_preserves_others) {
	char filename[64];
	strcpy(filename, "test_08_remove_accuracy.xpk");

	xpkObject xpk = xpkOpen(filename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	uint32_t hashes[5];
	size_t sizes[5];

	for (int i = 0; i < 5; i++) {
		sizes[i] = (i + 1) * 1000;
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

	xpk = xpkOpen(filename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkRemove(xpk, 2), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(filename, 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 4);

	for (int i = 0; i < 4; i++) {
		ASSERT_EQ(xpkInfoHash(xpk, i), hashes[i > 1 ? i + 1 : i]);
	}

	xpkClose(xpk);
}

TEST(accuracy_rebuild_preserves_data) {
	char filename[64];
	strcpy(filename, "test_08_rebuild_accuracy.xpk");

	xpkObject xpk = xpkOpen(filename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	uint32_t hashes[10];
	size_t sizes[10];

	for (int i = 0; i < 10; i++) {
		sizes[i] = (i + 1) * 500;
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

	xpk = xpkOpen(filename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkRebuild(xpk), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(filename, 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 10);

	for (int i = 0; i < 10; i++) {
		ASSERT_EQ(xpkInfoHash(xpk, i), hashes[i]);
	}

	xpkClose(xpk);
}

TEST(accuracy_mixed_levels_independent) {
	char filename[64];
	strcpy(filename, "test_08_mixed_levels.xpk");

	xpkObject xpk = xpkOpen(filename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	uint32_t hashes[8];
	size_t sizes[8];

	for (int i = 0; i < 8; i++) {
		sizes[i] = 2000;
		void* data = malloc(sizes[i]);
		ASSERT_NOT_NULL(data);

		srand((unsigned int)(time(NULL) + i));
		for (size_t j = 0; j < sizes[i]; j++) {
			((uint8_t*)data)[j] = (uint8_t)rand();
		}

		hashes[i] = xrtHash32((ptr)data, sizes[i]);

		int level = i * 2;
		xpkAppendData(xpk, data, (uint32_t)sizes[i], level);

		ASSERT_EQ(xpkInfoLevel(xpk, i), level);
		ASSERT_EQ(xpkInfoHash(xpk, i), hashes[i]);

		free(data);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);
}

void register_08_compression_accuracy_tests(void) {
    TEST_REGISTER(accuracy_all_levels_byte_for_byte, CAT_COMPRESSION, "Accuracy All Levels Byte For Byte");
    TEST_REGISTER(accuracy_hash_consistency, CAT_COMPRESSION, "Accuracy Hash Consistency");
    TEST_REGISTER(accuracy_multiple_files_independent, CAT_COMPRESSION, "Accuracy Multiple Files Independent");
    TEST_REGISTER(accuracy_compress_decompress_cycle, CAT_COMPRESSION, "Accuracy Compress Decompress Cycle");
    TEST_REGISTER(accuracy_file_operations, CAT_COMPRESSION, "Accuracy File Operations");
    TEST_REGISTER(accuracy_update_preserves_data, CAT_COMPRESSION, "Accuracy Update Preserves Data");
    TEST_REGISTER(accuracy_remove_preserves_others, CAT_COMPRESSION, "Accuracy Remove Preserves Others");
    TEST_REGISTER(accuracy_rebuild_preserves_data, CAT_COMPRESSION, "Accuracy Rebuild Preserves Data");
    TEST_REGISTER(accuracy_mixed_levels_independent, CAT_COMPRESSION, "Accuracy Mixed Levels Independent");
}
