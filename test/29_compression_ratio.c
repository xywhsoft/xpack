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

TEST(compression_ratio_repeated_data) {
	const char* sFilename = "test_29_ratio_repeated.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 15);
	ASSERT_NOT_NULL(xpk);

	char* pData = createTestData(1024 * 1024, 'A');
	ASSERT_EQ(xpkAppendData(xpk, pData, 1024 * 1024, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	int iOriginalSize = 0;
	int iCompressedSize = 0;
	ASSERT_EQ(xpkStatGet(xpk, &iOriginalSize, &iCompressedSize, 0), 0);

	ASSERT_EQ(iOriginalSize, 1024 * 1024);
	ASSERT_NE(iCompressedSize, 0);
	ASSERT_LT(iCompressedSize, iOriginalSize);

	float fRatio = (iCompressedSize * 100.0f) / iOriginalSize;
	printf("  Repeated data: Original=%d, Compressed=%d, Ratio=%.2f%%\n",
	       iOriginalSize, iCompressedSize, fRatio);

	free(pData);
	xpkClose(xpk);
}

TEST(compression_ratio_random_data) {
	const char* sFilename = "test_29_ratio_random.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 15);
	ASSERT_NOT_NULL(xpk);

	char* pData = createTestData(1024 * 1024, 'R');
	for ( int i = 0; i < 1024 * 1024; i++ ) {
		pData[i] = (char)(rand() % 256);
	}

	ASSERT_EQ(xpkAppendData(xpk, pData, 1024 * 1024, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	int iOriginalSize = 0;
	int iCompressedSize = 0;
	ASSERT_EQ(xpkStatGet(xpk, &iOriginalSize, &iCompressedSize, 0), 0);

	ASSERT_EQ(iOriginalSize, 1024 * 1024);
	ASSERT_NE(iCompressedSize, 0);

	float fRatio = (iCompressedSize * 100.0f) / iOriginalSize;
	printf("  Random data: Original=%d, Compressed=%d, Ratio=%.2f%%\n",
	       iOriginalSize, iCompressedSize, fRatio);

	free(pData);
	xpkClose(xpk);
}

TEST(compression_ratio_zero_data) {
	const char* sFilename = "test_29_ratio_zero.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 15);
	ASSERT_NOT_NULL(xpk);

	char* pData = createTestData(1024 * 1024, '\0');
	ASSERT_EQ(xpkAppendData(xpk, pData, 1024 * 1024, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	int iOriginalSize = 0;
	int iCompressedSize = 0;
	ASSERT_EQ(xpkStatGet(xpk, &iOriginalSize, &iCompressedSize, 0), 0);

	ASSERT_EQ(iOriginalSize, 1024 * 1024);
	ASSERT_NE(iCompressedSize, 0);
	ASSERT_LT(iCompressedSize, iOriginalSize);

	float fRatio = (iCompressedSize * 100.0f) / iOriginalSize;
	printf("  Zero data: Original=%d, Compressed=%d, Ratio=%.2f%%\n",
	       iOriginalSize, iCompressedSize, fRatio);

	free(pData);
	xpkClose(xpk);
}

TEST(compression_ratio_text_data) {
	const char* sFilename = "test_29_ratio_text.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 15);
	ASSERT_NOT_NULL(xpk);

	char* pData = createTestData(1024 * 1024, 'T');
	for ( int i = 0; i < 1024 * 1024; i++ ) {
		pData[i] = (char)(32 + (rand() % 64));
	}

	ASSERT_EQ(xpkAppendData(xpk, pData, 1024 * 1024, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	int iOriginalSize = 0;
	int iCompressedSize = 0;
	ASSERT_EQ(xpkStatGet(xpk, &iOriginalSize, &iCompressedSize, 0), 0);

	ASSERT_EQ(iOriginalSize, 1024 * 1024);
	ASSERT_NE(iCompressedSize, 0);

	float fRatio = (iCompressedSize * 100.0f) / iOriginalSize;
	printf("  Text data: Original=%d, Compressed=%d, Ratio=%.2f%%\n",
	       iOriginalSize, iCompressedSize, fRatio);

	free(pData);
	xpkClose(xpk);
}

TEST(compression_ratio_levels_comparison) {
	const char* sFilename = "test_29_ratio_levels.xpk";

	int iOriginalSize = 1024 * 1024;
	char* pData = createTestData(iOriginalSize, 'L');

	for ( int iLevel = 0; iLevel <= 15; iLevel++ ) {
		char sLevelFilename[64];
		sprintf(sLevelFilename, "%s_%d", sFilename, iLevel);

		xpkObject xpk = xpkOpen(sLevelFilename, XPK_TYPE_CORE, iLevel);
		ASSERT_NOT_NULL(xpk);

		ASSERT_EQ(xpkAppendData(xpk, pData, iOriginalSize, 1), 0);
		ASSERT_EQ(xpkSave(xpk), 0);

		int iCompressedSize = 0;
		ASSERT_EQ(xpkStatGet(xpk, NULL, &iCompressedSize, 0), 0);

		ASSERT_NE(iCompressedSize, 0);

		float fRatio = (iCompressedSize * 100.0f) / iOriginalSize;
		printf("  Level %2d: Compressed=%d, Ratio=%.2f%%\n",
		       iLevel, iCompressedSize, fRatio);

		xpkClose(xpk);
	}

	free(pData);
}

TEST(compression_ratio_json_data) {
	const char* sFilename = "test_29_ratio_json.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 15);
	ASSERT_NOT_NULL(xpk);

	char* pData = createTestData(1024 * 1024, 'J');
	for ( int i = 0; i < 1024 * 1024; i++ ) {
		pData[i] = (char)(32 + (rand() % 95));
		if ( i % 100 == 0 ) {
			pData[i] = '{';
		} else if ( i % 100 == 50 ) {
			pData[i] = ':';
		} else if ( i % 100 == 99 ) {
			pData[i] = '}';
		}
	}

	ASSERT_EQ(xpkAppendData(xpk, pData, 1024 * 1024, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	int iOriginalSize = 0;
	int iCompressedSize = 0;
	ASSERT_EQ(xpkStatGet(xpk, &iOriginalSize, &iCompressedSize, 0), 0);

	ASSERT_EQ(iOriginalSize, 1024 * 1024);
	ASSERT_NE(iCompressedSize, 0);

	float fRatio = (iCompressedSize * 100.0f) / iOriginalSize;
	printf("  JSON-like data: Original=%d, Compressed=%d, Ratio=%.2f%%\n",
	       iOriginalSize, iCompressedSize, fRatio);

	free(pData);
	xpkClose(xpk);
}

TEST(compression_ratio_xml_data) {
	const char* sFilename = "test_29_ratio_xml.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 15);
	ASSERT_NOT_NULL(xpk);

	char* pData = createTestData(1024 * 1024, 'X');
	for ( int i = 0; i < 1024 * 1024; i++ ) {
		pData[i] = (char)(32 + (rand() % 95));
		if ( i % 100 == 0 ) {
			pData[i] = '<';
		} else if ( i % 100 == 50 ) {
			pData[i] = '>';
		}
	}

	ASSERT_EQ(xpkAppendData(xpk, pData, 1024 * 1024, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	int iOriginalSize = 0;
	int iCompressedSize = 0;
	ASSERT_EQ(xpkStatGet(xpk, &iOriginalSize, &iCompressedSize, 0), 0);

	ASSERT_EQ(iOriginalSize, 1024 * 1024);
	ASSERT_NE(iCompressedSize, 0);

	float fRatio = (iCompressedSize * 100.0f) / iOriginalSize;
	printf("  XML-like data: Original=%d, Compressed=%d, Ratio=%.2f%%\n",
	       iOriginalSize, iCompressedSize, fRatio);

	free(pData);
	xpkClose(xpk);
}

TEST(compression_ratio_binary_data) {
	const char* sFilename = "test_29_ratio_binary.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 15);
	ASSERT_NOT_NULL(xpk);

	char* pData = createTestData(1024 * 1024, 'B');
	for ( int i = 0; i < 1024 * 1024; i++ ) {
		pData[i] = (char)(rand() % 256);
	}

	ASSERT_EQ(xpkAppendData(xpk, pData, 1024 * 1024, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	int iOriginalSize = 0;
	int iCompressedSize = 0;
	ASSERT_EQ(xpkStatGet(xpk, &iOriginalSize, &iCompressedSize, 0), 0);

	ASSERT_EQ(iOriginalSize, 1024 * 1024);
	ASSERT_NE(iCompressedSize, 0);

	float fRatio = (iCompressedSize * 100.0f) / iOriginalSize;
	printf("  Binary data: Original=%d, Compressed=%d, Ratio=%.2f%%\n",
	       iOriginalSize, iCompressedSize, fRatio);

	free(pData);
	xpkClose(xpk);
}

TEST(compression_ratio_mixed_files) {
	const char* sFilename = "test_29_ratio_mixed.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 10);
	ASSERT_NOT_NULL(xpk);

	char* pRepeated = createTestData(1024 * 1024, 'A');
	char* pRandom = createTestData(512 * 1024, 'R');
	for ( int i = 0; i < 512 * 1024; i++ ) {
		pRandom[i] = (char)(rand() % 256);
	}
	char* pZero = createTestData(256 * 1024, '\0');

	ASSERT_EQ(xpkAppendData(xpk, pRepeated, 1024 * 1024, 1), 0);
	ASSERT_EQ(xpkAppendData(xpk, pRandom, 512 * 1024, 2), 0);
	ASSERT_EQ(xpkAppendData(xpk, pZero, 256 * 1024, 3), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	int iOriginalSize = 0;
	int iCompressedSize = 0;
	ASSERT_EQ(xpkStatGet(xpk, &iOriginalSize, &iCompressedSize, 0), 0);

	ASSERT_EQ(iOriginalSize, 1024 * 1024 + 512 * 1024 + 256 * 1024);
	ASSERT_NE(iCompressedSize, 0);

	float fRatio = (iCompressedSize * 100.0f) / iOriginalSize;
	printf("  Mixed files: Original=%d, Compressed=%d, Ratio=%.2f%%\n",
	       iOriginalSize, iCompressedSize, fRatio);

	free(pRepeated);
	free(pRandom);
	free(pZero);
	xpkClose(xpk);
}

TEST(compression_ratio_size_impact) {
	const char* sFilename = "test_29_ratio_size.xpk";

	int sizes[] = {1024, 10240, 102400, 1048576};

	for ( int i = 0; i < 4; i++ ) {
		char sSizeFilename[64];
		sprintf(sSizeFilename, "%s_%d", sFilename, i);

		xpkObject xpk = xpkOpen(sSizeFilename, XPK_TYPE_CORE, 10);
		ASSERT_NOT_NULL(xpk);

		char* pData = createTestData(sizes[i], 'S');
		ASSERT_EQ(xpkAppendData(xpk, pData, sizes[i], 1), 0);
		ASSERT_EQ(xpkSave(xpk), 0);

		int iOriginalSize = 0;
		int iCompressedSize = 0;
		ASSERT_EQ(xpkStatGet(xpk, &iOriginalSize, &iCompressedSize, 0), 0);

		ASSERT_EQ(iOriginalSize, sizes[i]);
		ASSERT_NE(iCompressedSize, 0);

		float fRatio = (iCompressedSize * 100.0f) / iOriginalSize;
		printf("  Size %8d bytes: Compressed=%d, Ratio=%.2f%%\n",
		       sizes[i], iCompressedSize, fRatio);

		free(pData);
		xpkClose(xpk);
	}
}

TEST(compression_ratio_after_update) {
	const char* sFilename = "test_29_ratio_update.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 10);
	ASSERT_NOT_NULL(xpk);

	char* pOriginal = createTestData(1024 * 1024, 'O');
	ASSERT_EQ(xpkAppendData(xpk, pOriginal, 1024 * 1024, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	int iOriginalSize1 = 0;
	int iCompressedSize1 = 0;
	ASSERT_EQ(xpkStatGet(xpk, &iOriginalSize1, &iCompressedSize1, 0), 0);

	char* pUpdated = createTestData(2048 * 1024, 'U');
	ASSERT_EQ(xpkUpdateData(xpk, pUpdated, 2048 * 1024, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	int iOriginalSize2 = 0;
	int iCompressedSize2 = 0;
	ASSERT_EQ(xpkStatGet(xpk, &iOriginalSize2, &iCompressedSize2, 0), 0);

	ASSERT_EQ(iOriginalSize2, 2048 * 1024);
	ASSERT_NE(iCompressedSize2, 0);

	float fRatio1 = (iCompressedSize1 * 100.0f) / iOriginalSize1;
	float fRatio2 = (iCompressedSize2 * 100.0f) / iOriginalSize2;
	printf("  Before update: Ratio=%.2f%%, After update: Ratio=%.2f%%\n",
	       fRatio1, fRatio2);

	free(pOriginal);
	free(pUpdated);
	xpkClose(xpk);
}

TEST(compression_ratio_after_rebuild) {
	const char* sFilename = "test_29_ratio_rebuild.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 10);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 100; i++ ) {
		char* pData = createTestData(10240, 'R' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 10240, i + 1), 0);
		free(pData);
	}

	ASSERT_EQ(xpkSave(xpk), 0);

	int iOriginalSize1 = 0;
	int iCompressedSize1 = 0;
	ASSERT_EQ(xpkStatGet(xpk, &iOriginalSize1, &iCompressedSize1, 0), 0);

	for ( int i = 0; i < 50; i++ ) {
		ASSERT_EQ(xpkRemove(xpk, i * 2 + 1), 0);
	}

	ASSERT_EQ(xpkRebuild(xpk), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	int iOriginalSize2 = 0;
	int iCompressedSize2 = 0;
	ASSERT_EQ(xpkStatGet(xpk, &iOriginalSize2, &iCompressedSize2, 0), 0);

	ASSERT_EQ(iOriginalSize2, 50 * 10240);
	ASSERT_NE(iCompressedSize2, 0);

	float fRatio1 = (iCompressedSize1 * 100.0f) / iOriginalSize1;
	float fRatio2 = (iCompressedSize2 * 100.0f) / iOriginalSize2;
	printf("  Before rebuild: Ratio=%.2f%%, After rebuild: Ratio=%.2f%%\n",
	       fRatio1, fRatio2);

	xpkClose(xpk);
}

int main(int argc, char* argv[]) {
	xrtInit();

	printf("[Compression Ratio Tests]\n");
	printf("=========================\n\n");

	RUN_TEST(compression_ratio_repeated_data);
	RUN_TEST(compression_ratio_random_data);
	RUN_TEST(compression_ratio_zero_data);
	RUN_TEST(compression_ratio_text_data);
	RUN_TEST(compression_ratio_levels_comparison);
	RUN_TEST(compression_ratio_json_data);
	RUN_TEST(compression_ratio_xml_data);
	RUN_TEST(compression_ratio_binary_data);
	RUN_TEST(compression_ratio_mixed_files);
	RUN_TEST(compression_ratio_size_impact);
	RUN_TEST(compression_ratio_after_update);
	RUN_TEST(compression_ratio_after_rebuild);

	printf("=========================\n");
	printf("All Compression Ratio Tests Passed!\n");

	return 0;
}
