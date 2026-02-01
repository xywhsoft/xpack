/*
 * xPack Ver7 - save_load_cycles (19)
 */

#include "test_framework.h"
#include <stdlib.h>

static char* createTestData(int size, char pattern) {
	char* pData = (char*)malloc(size);
	for (int i = 0; i < size; i++) {
		pData[i] = (char)(pattern + (i % 26));
	}
	return pData;
}

TEST(single_save_load_cycle) {
	char* pData = createTestData(1024, 'A');
	const char* sFilename = "test_19_single_cycle.xpk";

	xpkObject xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_NE(xpkAppendData(xpk, pData, 1024, 1), UINT32_MAX);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	uint32_t outSize = 0;
	void* pExtracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(pExtracted);
	ASSERT_EQ(outSize, 1024);
	ASSERT_EQ(memcmp(pData, pExtracted, 1024), 0);
	xpkFree(pExtracted);

	free(pData);
	xpkClose(xpk);
}

TEST(multiple_save_load_cycles) {
	char* pData = createTestData(2048, 'B');
	const char* sFilename = "test_19_multiple_cycles.xpk";

	for (int iCycle = 0; iCycle < 5; iCycle++) {
		xpkObject xpk = xpkOpen(sFilename, 0, 0);
		ASSERT_NOT_NULL(xpk);

		if (iCycle > 0) {
			uint32_t outSize = 0;
			void* pExtracted = xpkExtractData(xpk, 0, &outSize);
			ASSERT_NOT_NULL(pExtracted);
			ASSERT_EQ(outSize, 2048);
			ASSERT_EQ(memcmp(pData, pExtracted, 2048), 0);
			xpkFree(pExtracted);
		}

		ASSERT_NE(xpkAppendData(xpk, pData, 2048, 6), UINT32_MAX);
		ASSERT_EQ(xpkSave(xpk), 0);
		xpkClose(xpk);
	}

	free(pData);
}

TEST(save_load_with_modifications) {
	char* pData1 = createTestData(1024, 'C');
	char* pData2 = createTestData(2048, 'D');
	const char* sFilename = "test_19_with_modifications.xpk";

	xpkObject xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_NE(xpkAppendData(xpk, pData1, 1024, 1), UINT32_MAX);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_NE(xpkAppendData(xpk, pData2, 2048, 2), UINT32_MAX);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	uint32_t outSize1 = 0;
	uint32_t outSize2 = 0;
	void* pExtracted1 = xpkExtractData(xpk, 0, &outSize1);
	void* pExtracted2 = xpkExtractData(xpk, 1, &outSize2);
	ASSERT_NOT_NULL(pExtracted1);
	ASSERT_NOT_NULL(pExtracted2);
	ASSERT_EQ(outSize1, 1024);
	ASSERT_EQ(outSize2, 2048);
	ASSERT_EQ(memcmp(pData1, pExtracted1, 1024), 0);
	ASSERT_EQ(memcmp(pData2, pExtracted2, 2048), 0);

	xpkFree(pExtracted1);
	xpkFree(pExtracted2);
	free(pData1);
	free(pData2);
	xpkClose(xpk);
}

TEST(save_load_after_remove) {
	char* pData1 = createTestData(512, 'E');
	char* pData2 = createTestData(1024, 'F');
	char* pData3 = createTestData(2048, 'G');
	const char* sFilename = "test_19_after_remove.xpk";

	xpkObject xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_NE(xpkAppendData(xpk, pData1, 512, 1), UINT32_MAX);
	ASSERT_NE(xpkAppendData(xpk, pData2, 1024, 2), UINT32_MAX);
	ASSERT_NE(xpkAppendData(xpk, pData3, 2048, 3), UINT32_MAX);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	// Remove middle file (pData2 at position 1)
	ASSERT_EQ(xpkRemove(xpk, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	// After removing position 1, pData3 shifts from position 2 to position 1
	uint32_t outSize1 = 0;
	uint32_t outSize2 = 0;
	void* pExtracted1 = xpkExtractData(xpk, 0, &outSize1);
	void* pExtracted2 = xpkExtractData(xpk, 1, &outSize2);
	ASSERT_NOT_NULL(pExtracted1);
	ASSERT_NOT_NULL(pExtracted2);
	ASSERT_EQ(outSize1, 512);
	ASSERT_EQ(outSize2, 2048);  // pData3 is now at position 1
	ASSERT_EQ(memcmp(pData1, pExtracted1, 512), 0);
	ASSERT_EQ(memcmp(pData3, pExtracted2, 2048), 0);

	xpkFree(pExtracted1);
	xpkFree(pExtracted2);
	free(pData1);
	free(pData2);
	free(pData3);
	xpkClose(xpk);
}

TEST(save_load_after_update) {
	char* pData = createTestData(1024, 'H');
	char* pNewData = createTestData(2048, 'I');
	const char* sFilename = "test_19_after_update.xpk";

	xpkObject xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_NE(xpkAppendData(xpk, pData, 1024, 1), UINT32_MAX);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkUpdateData(xpk, 0, pNewData, 2048, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	uint32_t outSize = 0;
	void* pExtracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(pExtracted);
	ASSERT_EQ(outSize, 2048);
	ASSERT_EQ(memcmp(pNewData, pExtracted, 2048), 0);

	xpkFree(pExtracted);
	free(pData);
	free(pNewData);
	xpkClose(xpk);
}

TEST(save_load_different_compression_levels) {
	char* pData = createTestData(8192, 'J');

	for (int iLevel = 0; iLevel <= 9; iLevel++) {
		// Use unique filename for each compression level to avoid file accumulation
		char sFilename[64];
		sprintf(sFilename, "test_19_compression_level_%d.xpk", iLevel);

		xpkObject xpk = xpkOpen(sFilename, 0, 0);
		ASSERT_NOT_NULL(xpk);

		ASSERT_NE(xpkAppendData(xpk, pData, 8192, iLevel), UINT32_MAX);
		ASSERT_EQ(xpkSave(xpk), 0);
		xpkClose(xpk);

		xpk = xpkOpen(sFilename, 0, 0);
		ASSERT_NOT_NULL(xpk);

		uint32_t outSize = 0;
		void* pExtracted = xpkExtractData(xpk, 0, &outSize);
		ASSERT_NOT_NULL(pExtracted);
		ASSERT_EQ(outSize, 8192);
		ASSERT_EQ(memcmp(pData, pExtracted, 8192), 0);
		xpkFree(pExtracted);
		xpkClose(xpk);
	}

	free(pData);
}

TEST(save_load_index_mode) {
	char* pData1 = createTestData(1024, 'K');
	char* pData2 = createTestData(2048, 'L');
	const char* sFilename = "test_19_index_mode.xpk";

	xpkObject xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);
	// xpkIndexAppendData returns pointer, not 0 for success
	ASSERT_NOT_NULL(xpkIndexAppendData(xpk, 100, pData1, 1024, 5));
	ASSERT_NOT_NULL(xpkIndexAppendData(xpk, 200, pData2, 2048, 5));
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	uint32_t outSize1 = 0;
	uint32_t outSize2 = 0;
	void* pExtracted1 = xpkExtractData(xpk, 0, &outSize1);
	void* pExtracted2 = xpkExtractData(xpk, 1, &outSize2);
	ASSERT_NOT_NULL(pExtracted1);
	ASSERT_NOT_NULL(pExtracted2);
	ASSERT_EQ(outSize1, 1024);
	ASSERT_EQ(outSize2, 2048);
	ASSERT_EQ(memcmp(pData1, pExtracted1, 1024), 0);
	ASSERT_EQ(memcmp(pData2, pExtracted2, 2048), 0);

	xpkFree(pExtracted1);
	xpkFree(pExtracted2);
	free(pData1);
	free(pData2);
	xpkClose(xpk);
}

TEST(save_load_path_mode) {
	char* pData1 = createTestData(1024, 'M');
	char* pData2 = createTestData(2048, 'N');
	const char* sFilename = "test_19_path_mode.xpk";

	xpkObject xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);
	// xpkPathAppendData returns position, not 0 for success
	ASSERT_NE(xpkPathAppendData(xpk, "file1.txt", pData1, 1024, 5), UINT32_MAX);
	ASSERT_NE(xpkPathAppendData(xpk, "file2.txt", pData2, 2048, 5), UINT32_MAX);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	uint32_t outSize1 = 0;
	uint32_t outSize2 = 0;
	void* pExtracted1 = xpkExtractData(xpk, 0, &outSize1);
	void* pExtracted2 = xpkExtractData(xpk, 1, &outSize2);
	ASSERT_NOT_NULL(pExtracted1);
	ASSERT_NOT_NULL(pExtracted2);
	ASSERT_EQ(outSize1, 1024);
	ASSERT_EQ(outSize2, 2048);
	ASSERT_EQ(memcmp(pData1, pExtracted1, 1024), 0);
	ASSERT_EQ(memcmp(pData2, pExtracted2, 2048), 0);

	xpkFree(pExtracted1);
	xpkFree(pExtracted2);
	free(pData1);
	free(pData2);
	xpkClose(xpk);
}

TEST(save_load_empty_package) {
	const char* sFilename = "test_19_empty.xpk";

	xpkObject xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 0);
	xpkClose(xpk);
}

TEST(save_load_large_file) {
	char* pData = createTestData(5 * 1024 * 1024, 'P');
	const char* sFilename = "test_19_large_file.xpk";

	xpkObject xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_NE(xpkAppendData(xpk, pData, 5 * 1024 * 1024, 6), UINT32_MAX);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	uint32_t outSize = 0;
	void* pExtracted = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(pExtracted);
	ASSERT_EQ(outSize, 5 * 1024 * 1024);
	ASSERT_EQ(memcmp(pData, pExtracted, 5 * 1024 * 1024), 0);

	xpkFree(pExtracted);
	free(pData);
	xpkClose(xpk);
}

void register_19_save_load_cycles_tests(void) {
	TEST_REGISTER(single_save_load_cycle, CAT_CYCLE, "Single save load cycle");
	TEST_REGISTER(multiple_save_load_cycles, CAT_CYCLE, "Multiple save load cycles");
	TEST_REGISTER(save_load_with_modifications, CAT_CYCLE, "Save load with modifications");
	TEST_REGISTER(save_load_after_remove, CAT_CYCLE, "Save load after remove");
	TEST_REGISTER(save_load_after_update, CAT_CYCLE, "Save load after update");
	TEST_REGISTER(save_load_different_compression_levels, CAT_CYCLE, "Save load different compression levels");
	TEST_REGISTER(save_load_index_mode, CAT_CYCLE, "Save load index mode");
	TEST_REGISTER(save_load_path_mode, CAT_CYCLE, "Save load path mode");
	TEST_REGISTER(save_load_empty_package, CAT_CYCLE, "Save load empty package");
	TEST_REGISTER(save_load_large_file, CAT_CYCLE, "Save load large file");
}
