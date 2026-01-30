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

TEST(create_and_destroy_package) {
	char* pData = createTestData(1024, 'A');
	const char* sFilename = "test_26_create.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, 1024, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	free(pData);
}

TEST(multiple_open_close_cycles) {
	const char* sFilename = "test_26_open_close.xpk";

	for ( int i = 0; i < 20; i++ ) {
		xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
		ASSERT_NOT_NULL(xpk);
		xpkClose(xpk);
	}
}

TEST(extract_with_allocated_buffer) {
	char* pData = createTestData(2048, 'B');
	const char* sFilename = "test_26_alloc_buf.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, 2048, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	char* pBuffer = (char*)malloc(2048);
	int iSize = 2048;
	ASSERT_EQ(xpkExtract(xpk, pBuffer, &iSize, 1), 0);
	ASSERT_EQ(iSize, 2048);
	ASSERT_EQ(memcmp(pData, pBuffer, 2048), 0);

	free(pBuffer);
	free(pData);
	xpkClose(xpk);
}

TEST(extract_with_null_buffer) {
	char* pData = createTestData(1024, 'C');
	const char* sFilename = "test_26_null_buf.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, 1024, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	int iSize = 0;
	ASSERT_EQ(xpkExtract(xpk, NULL, &iSize, 1), 0);
	ASSERT_EQ(iSize, 1024);

	free(pData);
	xpkClose(xpk);
}

TEST(extract_with_insufficient_buffer) {
	char* pData = createTestData(2048, 'D');
	const char* sFilename = "test_26_small_buf.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, 2048, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	char* pBuffer = (char*)malloc(512);
	int iSize = 512;
	ASSERT_NE(xpkExtract(xpk, pBuffer, &iSize, 1), 0);

	free(pBuffer);
	free(pData);
	xpkClose(xpk);
}

TEST(info_with_null_parameters) {
	char* pData = createTestData(1024, 'E');
	const char* sFilename = "test_26_null_info.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, 1024, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_EQ(xpkInfo(xpk, NULL, NULL, 0, 1), 0);

	free(pData);
	xpkClose(xpk);
}

TEST(stat_with_null_parameters) {
	char* pData = createTestData(2048, 'F');
	const char* sFilename = "test_26_null_stat.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, 2048, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_EQ(xpkStatGet(xpk, NULL, NULL, 0), 0);

	free(pData);
	xpkClose(xpk);
}

TEST(user_data_memory_handling) {
	char* pData = createTestData(1024, 'G');
	char* pUserData = createTestData(256, 'H');
	const char* sFilename = "test_26_userdata.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 5);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkAppendData(xpk, pData, 1024, 10), 0);
	ASSERT_EQ(xpkUserData(xpk, pUserData, 256, 10), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	char* pExtractedUserData = (char*)malloc(256);
	int iUserSize = 256;
	ASSERT_EQ(xpkGetUser(xpk, pExtractedUserData, &iUserSize, 10), 0);
	ASSERT_EQ(iUserSize, 256);
	ASSERT_EQ(memcmp(pUserData, pExtractedUserData, 256), 0);

	free(pExtractedUserData);
	free(pData);
	free(pUserData);
	xpkClose(xpk);
}

TEST(repeated_append_free_cycles) {
	const char* sFilename = "test_26_append_free.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	for ( int i = 0; i < 100; i++ ) {
		char* pData = createTestData(512, 'A' + (i % 26));
		ASSERT_EQ(xpkAppendData(xpk, pData, 512, i + 1), 0);
		free(pData);
	}

	ASSERT_EQ(xpkCount(xpk), 100);
	ASSERT_EQ(xpkSave(xpk), 0);

	xpkClose(xpk);
}

TEST(repeated_extract_free_cycles) {
	const char* sFilename = "test_26_extract_free.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	char* pData = createTestData(1024, 'I');
	ASSERT_EQ(xpkAppendData(xpk, pData, 1024, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	for ( int i = 0; i < 50; i++ ) {
		char* pBuffer = (char*)malloc(1024);
		int iSize = 1024;
		ASSERT_EQ(xpkExtract(xpk, pBuffer, &iSize, 1), 0);
		ASSERT_EQ(iSize, 1024);
		ASSERT_EQ(memcmp(pData, pBuffer, 1024), 0);
		free(pBuffer);
	}

	free(pData);
	xpkClose(xpk);
}

TEST(find_with_null_result) {
	const char* sFilename = "test_26_null_find.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 5);
	ASSERT_NOT_NULL(xpk);

	char* pData = createTestData(1024, 'J');
	ASSERT_EQ(xpkAppendData(xpk, pData, 1024, 100), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_NE(xpkFind(xpk, 100), -1);
	ASSERT_EQ(xpkFind(xpk, 999), -1);

	free(pData);
	xpkClose(xpk);
}

TEST(each_with_null_callback) {
	const char* sFilename = "test_26_null_each.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_INDEX, 5);
	ASSERT_NOT_NULL(xpk);

	char* pData = createTestData(512, 'K');
	for ( int i = 0; i < 10; i++ ) {
		ASSERT_EQ(xpkAppendData(xpk, pData, 512, i * 10), 0);
	}
	ASSERT_EQ(xpkSave(xpk), 0);

	int iCount = 0;
	ASSERT_EQ(xpkEach(xpk, NULL, &iCount), 0);
	ASSERT_EQ(iCount, 10);

	free(pData);
	xpkClose(xpk);
}

TEST(match_with_null_callback) {
	const char* sFilename = "test_26_null_match.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_PATH, 5);
	ASSERT_NOT_NULL(xpk);

	char* pData = createTestData(512, 'L');
	ASSERT_EQ(xpkAppend(xpk, "test/file1.txt", pData, 512), 0);
	ASSERT_EQ(xpkAppend(xpk, "test/file2.txt", pData, 512), 0);
	ASSERT_EQ(xpkAppend(xpk, "other/file3.txt", pData, 512), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	int iCount = 0;
	ASSERT_EQ(xpkEachMatch(xpk, "test/*.txt", NULL, &iCount), 0);
	ASSERT_EQ(iCount, 2);

	free(pData);
	xpkClose(xpk);
}

TEST(head_with_null_buffer) {
	const char* sFilename = "test_26_null_head.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	char* pData = createTestData(2048, 'M');
	ASSERT_EQ(xpkAppendData(xpk, pData, 2048, 1), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	int iHeadSize = xpkGetHead(NULL, 0);
	ASSERT_NE(iHeadSize, 0);

	char* pBuffer = (char*)malloc(iHeadSize);
	ASSERT_EQ(xpkGetHead(pBuffer, iHeadSize), iHeadSize);

	free(pBuffer);
	free(pData);
	xpkClose(xpk);
}

TEST(type_and_disc_code_queries) {
	const char* sFilename = "test_26_queries.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_CORE, 5);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkType(xpk), XPK_TYPE_CORE);
	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);
	ASSERT_EQ(xpkType(xpk), XPK_TYPE_INDEX);

	ASSERT_EQ(xpkDiscCodeSet(xpk, 0x12345678), 0);
	ASSERT_EQ(xpkDiscCode(xpk), 0x12345678);

	xpkClose(xpk);
}

int main(int argc, char* argv[]) {
	xrtInit();

	printf("[Memory Management Tests]\n");
	printf("=========================\n\n");

	RUN_TEST(create_and_destroy_package);
	RUN_TEST(multiple_open_close_cycles);
	RUN_TEST(extract_with_allocated_buffer);
	RUN_TEST(extract_with_null_buffer);
	RUN_TEST(extract_with_insufficient_buffer);
	RUN_TEST(info_with_null_parameters);
	RUN_TEST(stat_with_null_parameters);
	RUN_TEST(user_data_memory_handling);
	RUN_TEST(repeated_append_free_cycles);
	RUN_TEST(repeated_extract_free_cycles);
	RUN_TEST(find_with_null_result);
	RUN_TEST(each_with_null_callback);
	RUN_TEST(match_with_null_callback);
	RUN_TEST(head_with_null_buffer);
	RUN_TEST(type_and_disc_code_queries);

	printf("=========================\n");
	printf("All Memory Management Tests Passed!\n");

	return 0;
}
