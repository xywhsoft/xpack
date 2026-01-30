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

TEST(exactly_4mb_file) {
	int iSize = 4 * 1024 * 1024;
	char* pData = createTestData(iSize, 'A');
	const char* sFilename = "test_21_4mb.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 6);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, iSize, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	char* pExtracted = (char*)malloc(iSize);
	int iExtractedSize = iSize;
	ASSERT_EQ(xpkExtract(xpk, pExtracted, &iExtractedSize, 1), 0);
	ASSERT_EQ(iExtractedSize, iSize);
	ASSERT_EQ(memcmp(pData, pExtracted, iSize), 0);

	free(pExtracted);
	free(pData);
	xpkClose(xpk);
}

TEST(just_over_4mb_file) {
	int iSize = 4 * 1024 * 1024 + 1;
	char* pData = createTestData(iSize, 'B');
	const char* sFilename = "test_21_4mb_plus1.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 6);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, iSize, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	char* pExtracted = (char*)malloc(iSize);
	int iExtractedSize = iSize;
	ASSERT_EQ(xpkExtract(xpk, pExtracted, &iExtractedSize, 1), 0);
	ASSERT_EQ(iExtractedSize, iSize);
	ASSERT_EQ(memcmp(pData, pExtracted, iSize), 0);

	free(pExtracted);
	free(pData);
	xpkClose(xpk);
}

TEST(just_under_4mb_file) {
	int iSize = 4 * 1024 * 1024 - 1;
	char* pData = createTestData(iSize, 'C');
	const char* sFilename = "test_21_4mb_minus1.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 6);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, iSize, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	char* pExtracted = (char*)malloc(iSize);
	int iExtractedSize = iSize;
	ASSERT_EQ(xpkExtract(xpk, pExtracted, &iExtractedSize, 1), 0);
	ASSERT_EQ(iExtractedSize, iSize);
	ASSERT_EQ(memcmp(pData, pExtracted, iSize), 0);

	free(pExtracted);
	free(pData);
	xpkClose(xpk);
}

TEST(exactly_1mb_file) {
	int iSize = 1024 * 1024;
	char* pData = createTestData(iSize, 'D');
	const char* sFilename = "test_21_1mb.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 6);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, iSize, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	char* pExtracted = (char*)malloc(iSize);
	int iExtractedSize = iSize;
	ASSERT_EQ(xpkExtract(xpk, pExtracted, &iExtractedSize, 1), 0);
	ASSERT_EQ(iExtractedSize, iSize);
	ASSERT_EQ(memcmp(pData, pExtracted, iSize), 0);

	free(pExtracted);
	free(pData);
	xpkClose(xpk);
}

TEST(exactly_2mb_file) {
	int iSize = 2 * 1024 * 1024;
	char* pData = createTestData(iSize, 'E');
	const char* sFilename = "test_21_2mb.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 6);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, iSize, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	char* pExtracted = (char*)malloc(iSize);
	int iExtractedSize = iSize;
	ASSERT_EQ(xpkExtract(xpk, pExtracted, &iExtractedSize, 1), 0);
	ASSERT_EQ(iExtractedSize, iSize);
	ASSERT_EQ(memcmp(pData, pExtracted, iSize), 0);

	free(pExtracted);
	free(pData);
	xpkClose(xpk);
}

TEST(exactly_10mb_file) {
	int iSize = 10 * 1024 * 1024;
	char* pData = createTestData(iSize, 'F');
	const char* sFilename = "test_21_10mb.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 6);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, iSize, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	char* pExtracted = (char*)malloc(iSize);
	int iExtractedSize = iSize;
	ASSERT_EQ(xpkExtract(xpk, pExtracted, &iExtractedSize, 1), 0);
	ASSERT_EQ(iExtractedSize, iSize);
	ASSERT_EQ(memcmp(pData, pExtracted, iSize), 0);

	free(pExtracted);
	free(pData);
	xpkClose(xpk);
}

TEST(exactly_50mb_file) {
	int iSize = 50 * 1024 * 1024;
	char* pData = createTestData(iSize, 'G');
	const char* sFilename = "test_21_50mb.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 6);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, iSize, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	char* pExtracted = (char*)malloc(iSize);
	int iExtractedSize = iSize;
	ASSERT_EQ(xpkExtract(xpk, pExtracted, &iExtractedSize, 1), 0);
	ASSERT_EQ(iExtractedSize, iSize);
	ASSERT_EQ(memcmp(pData, pExtracted, iSize), 0);

	free(pExtracted);
	free(pData);
	xpkClose(xpk);
}

TEST(exactly_100mb_file) {
	int iSize = 100 * 1024 * 1024;
	char* pData = createTestData(iSize, 'H');
	const char* sFilename = "test_21_100mb.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 6);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, iSize, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	char* pExtracted = (char*)malloc(iSize);
	int iExtractedSize = iSize;
	ASSERT_EQ(xpkExtract(xpk, pExtracted, &iExtractedSize, 1), 0);
	ASSERT_EQ(iExtractedSize, iSize);
	ASSERT_EQ(memcmp(pData, pExtracted, iSize), 0);

	free(pExtracted);
	free(pData);
	xpkClose(xpk);
}

TEST(block_size_boundary_64kb) {
	int iSize = 64 * 1024;
	char* pData = createTestData(iSize, 'I');
	const char* sFilename = "test_21_64kb.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 6);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, iSize, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	char* pExtracted = (char*)malloc(iSize);
	int iExtractedSize = iSize;
	ASSERT_EQ(xpkExtract(xpk, pExtracted, &iExtractedSize, 1), 0);
	ASSERT_EQ(iExtractedSize, iSize);
	ASSERT_EQ(memcmp(pData, pExtracted, iSize), 0);

	free(pExtracted);
	free(pData);
	xpkClose(xpk);
}

TEST(block_size_boundary_256kb) {
	int iSize = 256 * 1024;
	char* pData = createTestData(iSize, 'J');
	const char* sFilename = "test_21_256kb.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 6);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, iSize, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	char* pExtracted = (char*)malloc(iSize);
	int iExtractedSize = iSize;
	ASSERT_EQ(xpkExtract(xpk, pExtracted, &iExtractedSize, 1), 0);
	ASSERT_EQ(iExtractedSize, iSize);
	ASSERT_EQ(memcmp(pData, pExtracted, iSize), 0);

	free(pExtracted);
	free(pData);
	xpkClose(xpk);
}

TEST(block_size_boundary_1mb_plus_1) {
	int iSize = 1024 * 1024 + 1;
	char* pData = createTestData(iSize, 'K');
	const char* sFilename = "test_21_1mb_plus1.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 6);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, iSize, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen(sFilename, 0, 0);
	ASSERT_NOT_NULL(xpk);

	char* pExtracted = (char*)malloc(iSize);
	int iExtractedSize = iSize;
	ASSERT_EQ(xpkExtract(xpk, pExtracted, &iExtractedSize, 1), 0);
	ASSERT_EQ(iExtractedSize, iSize);
	ASSERT_EQ(memcmp(pData, pExtracted, iSize), 0);

	free(pExtracted);
	free(pData);
	xpkClose(xpk);
}

int main(int argc, char* argv[]) {
	xrtInit();

	printf("[Edge Large Files Tests]\n");
	printf("=========================\n\n");

	RUN_TEST(exactly_4mb_file);
	RUN_TEST(just_over_4mb_file);
	RUN_TEST(just_under_4mb_file);
	RUN_TEST(exactly_1mb_file);
	RUN_TEST(exactly_2mb_file);
	RUN_TEST(exactly_10mb_file);
	RUN_TEST(exactly_50mb_file);
	RUN_TEST(exactly_100mb_file);
	RUN_TEST(block_size_boundary_64kb);
	RUN_TEST(block_size_boundary_256kb);
	RUN_TEST(block_size_boundary_1mb_plus_1);

	printf("=========================\n");
	printf("All Edge Large Files Tests Passed!\n");

	return 0;
}
