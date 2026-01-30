/*
 * xPack Ver7 - Index 模式操作测试
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

TEST(index_append_find) {
	xpkObject xpk = xpkOpen("test_04_append_find.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);
	ASSERT_EQ(xpkType(xpk), XPK_TYPE_INDEX);

	xpkFileInfoIndex* info1 = xpkIndexAppendData(xpk, 100, "Data 100", 8, 6);
	ASSERT_NOT_NULL(info1);
	ASSERT_EQ(info1->fileIndex, 100);

	xpkFileInfoIndex* info2 = xpkIndexAppendData(xpk, 200, "Data 200", 8, 6);
	ASSERT_NOT_NULL(info2);
	ASSERT_EQ(info2->fileIndex, 200);

	xpkFileInfoIndex* info3 = xpkIndexAppendData(xpk, 50, "Data 50", 7, 6);
	ASSERT_NOT_NULL(info3);
	ASSERT_EQ(info3->fileIndex, 50);

	ASSERT_EQ(xpkCount(xpk), 3);

	uint32_t pos = xpkIndexFind(xpk, 100);
	ASSERT_NE(pos, UINT32_MAX);
	ASSERT_EQ(pos, 0);

	pos = xpkIndexFind(xpk, 200);
	ASSERT_NE(pos, UINT32_MAX);
	ASSERT_EQ(pos, 1);

	pos = xpkIndexFind(xpk, 50);
	ASSERT_NE(pos, UINT32_MAX);
	ASSERT_EQ(pos, 2);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);
}

TEST(index_duplicate_detection) {
	xpkObject xpk = xpkOpen("test_04_duplicate.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

	xpkIndexAppendData(xpk, 100, "First", 5, 6);

	xpkFileInfoIndex* duplicate = xpkIndexAppendData(xpk, 100, "Second", 6, 6);
	ASSERT_NULL(duplicate);
	ASSERT_NE(xpkLastError(), 0);

	ASSERT_EQ(xpkCount(xpk), 1);

	xpkClose(xpk);
}

TEST(index_extract_file) {
	xpkObject xpk = xpkOpen("test_04_extract.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

	char srcPath[256];
	sprintf(srcPath, "test_src_04.txt");
	FILE* fp = fopen(srcPath, "wb");
	fwrite("Index file content", 18, 1, fp);
	fclose(fp);

	xpkIndexAppendFile(xpk, 42, srcPath, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_04_extract.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	char dstPath[256];
	sprintf(dstPath, "test_dst_04.txt");
	ASSERT_EQ(xpkIndexExtractFile(xpk, 42, dstPath), 0);

	fp = fopen(dstPath, "rb");
	char readData[64];
	size_t readSize = fread(readData, 1, sizeof(readData), fp);
	fclose(fp);

	ASSERT_EQ(readSize, 18);
	ASSERT_EQ(memcmp(readData, "Index file content", 18), 0);

	xpkClose(xpk);

	xrtFileDelete(srcPath);
	xrtFileDelete(dstPath);
}

TEST(index_extract_data) {
	xpkObject xpk = xpkOpen("test_04_extract_data.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

	xpkIndexAppendData(xpk, 123, "Test data", 9, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_04_extract_data.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t outSize = 0;
	void* data = xpkIndexExtractData(xpk, 123, &outSize);
	ASSERT_NOT_NULL(data);
	ASSERT_EQ(outSize, 9);
	ASSERT_EQ(memcmp(data, "Test data", 9), 0);
	xpkFree(data);

	data = xpkIndexExtractData(xpk, 999, &outSize);
	ASSERT_NULL(data);
	ASSERT_NE(xpkLastError(), 0);

	xpkClose(xpk);
}

TEST(index_update_file) {
	xpkObject xpk = xpkOpen("test_04_update.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

	xpkIndexAppendData(xpk, 100, "Original", 8, 6);

	char srcPath[256];
	sprintf(srcPath, "test_src_04_update.txt");
	FILE* fp = fopen(srcPath, "wb");
	fwrite("Updated content", 15, 1, fp);
	fclose(fp);

	ASSERT_EQ(xpkIndexUpdateFile(xpk, 100, srcPath, 6), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_04_update.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t outSize = 0;
	void* data = xpkIndexExtractData(xpk, 100, &outSize);
	ASSERT_NOT_NULL(data);
	ASSERT_EQ(outSize, 15);
	ASSERT_EQ(memcmp(data, "Updated content", 15), 0);
	xpkFree(data);

	xpkClose(xpk);

	xrtFileDelete(srcPath);
}

TEST(index_update_data) {
	xpkObject xpk = xpkOpen("test_04_update_data.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

	xpkIndexAppendData(xpk, 200, "Initial data", 12, 6);

	ASSERT_EQ(xpkIndexUpdateData(xpk, 200, "Modified data", 13, 6), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_04_update_data.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t outSize = 0;
	void* data = xpkIndexExtractData(xpk, 200, &outSize);
	ASSERT_NOT_NULL(data);
	ASSERT_EQ(outSize, 13);
	ASSERT_EQ(memcmp(data, "Modified data", 13), 0);
	xpkFree(data);

	xpkClose(xpk);
}

TEST(index_remove) {
	xpkObject xpk = xpkOpen("test_04_remove.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

	xpkIndexAppendData(xpk, 10, "Data 10", 7, 6);
	xpkIndexAppendData(xpk, 20, "Data 20", 7, 6);
	xpkIndexAppendData(xpk, 30, "Data 30", 7, 6);

	ASSERT_EQ(xpkIndexRemove(xpk, 20), 0);
	ASSERT_EQ(xpkCount(xpk), 2);
	ASSERT_EQ(xpkIndexFind(xpk, 20), UINT32_MAX);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_04_remove.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 2);

	ASSERT_NE(xpkIndexFind(xpk, 10), UINT32_MAX);
	ASSERT_NE(xpkIndexFind(xpk, 30), UINT32_MAX);
	ASSERT_EQ(xpkIndexFind(xpk, 20), UINT32_MAX);

	xpkClose(xpk);
}

TEST(index_user_data) {
	xpkObject xpk = xpkOpen("test_04_userdata.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

	xpkIndexAppendData(xpk, 100, "Data", 4, 6);

	ASSERT_EQ(xpkIndexUserDataSet(xpk, 100, 12345), 0);
	ASSERT_EQ(xpkIndexUserData(xpk, 100), 12345);

	ASSERT_EQ(xpkIndexUserDataSet(xpk, 100, -67890), 0);
	ASSERT_EQ(xpkIndexUserData(xpk, 100), -67890);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_04_userdata.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkIndexUserData(xpk, 100), -67890);

	xpkClose(xpk);
}

TEST(index_negative_index) {
	xpkObject xpk = xpkOpen("test_04_negative.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

	xpkIndexAppendData(xpk, -1, "Negative one", 12, 6);
	xpkIndexAppendData(xpk, -100, "Negative hundred", 15, 6);
	xpkIndexAppendData(xpk, 0, "Zero", 4, 6);

	ASSERT_EQ(xpkCount(xpk), 3);

	ASSERT_NE(xpkIndexFind(xpk, -1), UINT32_MAX);
	ASSERT_NE(xpkIndexFind(xpk, -100), UINT32_MAX);
	ASSERT_NE(xpkIndexFind(xpk, 0), UINT32_MAX);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_04_negative.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_NE(xpkIndexFind(xpk, -1), UINT32_MAX);
	ASSERT_NE(xpkIndexFind(xpk, -100), UINT32_MAX);
	ASSERT_NE(xpkIndexFind(xpk, 0), UINT32_MAX);

	xpkClose(xpk);
}

TEST(index_large_index) {
	xpkObject xpk = xpkOpen("test_04_large_index.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

	int32_t indices[] = { 1000000, 999999999, -1000000, 2147483647, -2147483648 };
	const char* data[] = { "Data 1", "Data 2", "Data 3", "Data 4", "Data 5" };
	size_t sizes[] = { 6, 6, 6, 6, 6 };

	for (int i = 0; i < 5; i++) {
		xpkIndexAppendData(xpk, indices[i], data[i], sizes[i], 6);
	}

	ASSERT_EQ(xpkCount(xpk), 5);

	for (int i = 0; i < 5; i++) {
		ASSERT_NE(xpkIndexFind(xpk, indices[i]), UINT32_MAX);

		uint32_t outSize = 0;
		void* extracted = xpkIndexExtractData(xpk, indices[i], &outSize);
		ASSERT_NOT_NULL(extracted);
		ASSERT_EQ(outSize, sizes[i]);
		ASSERT_EQ(memcmp(extracted, data[i], sizes[i]), 0);
		xpkFree(extracted);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);
}

TEST(index_order_preservation) {
	xpkObject xpk = xpkOpen("test_04_order.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

	xpkIndexAppendData(xpk, 50, "Fifty", 5, 6);
	xpkIndexAppendData(xpk, 10, "Ten", 3, 6);
	xpkIndexAppendData(xpk, 100, "Hundred", 7, 6);
	xpkIndexAppendData(xpk, 1, "One", 3, 6);

	ASSERT_EQ(xpkCount(xpk), 4);

	ASSERT_EQ(xpkIndexFind(xpk, 50), 0);
	ASSERT_EQ(xpkIndexFind(xpk, 10), 1);
	ASSERT_EQ(xpkIndexFind(xpk, 100), 2);
	ASSERT_EQ(xpkIndexFind(xpk, 1), 3);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_04_order.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkIndexFind(xpk, 50), 0);
	ASSERT_EQ(xpkIndexFind(xpk, 10), 1);
	ASSERT_EQ(xpkIndexFind(xpk, 100), 2);
	ASSERT_EQ(xpkIndexFind(xpk, 1), 3);

	xpkClose(xpk);
}

TEST(index_sparse_indices) {
	xpkObject xpk = xpkOpen("test_04_sparse.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

	for (int i = 0; i < 10; i++) {
		char data[32];
		sprintf(data, "Data %d", i * 100);
		xpkIndexAppendData(xpk, i * 100, data, (uint32_t)strlen(data), 6);
	}

	ASSERT_EQ(xpkCount(xpk), 10);

	ASSERT_EQ(xpkIndexFind(xpk, 0), 0);
	ASSERT_EQ(xpkIndexFind(xpk, 100), 1);
	ASSERT_EQ(xpkIndexFind(xpk, 500), 5);
	ASSERT_EQ(xpkIndexFind(xpk, 900), 9);

	ASSERT_EQ(xpkIndexFind(xpk, 50), UINT32_MAX);
	ASSERT_EQ(xpkIndexFind(xpk, 150), UINT32_MAX);
	ASSERT_EQ(xpkIndexFind(xpk, 1000), UINT32_MAX);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);
}

TEST(index_operations_on_non_index) {
	xpkObject xpk = xpkOpen("test_04_non_index.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkType(xpk), XPK_TYPE_CORE);

	ASSERT_EQ(xpkIndexFind(xpk, 100), UINT32_MAX);
	ASSERT_NULL(xpkIndexAppendData(xpk, 100, "Data", 4, 6));
	ASSERT_NULL(xpkIndexExtractData(xpk, 100, NULL));
	ASSERT_NE(xpkIndexUpdateData(xpk, 100, "New", 3, 6), 0);
	ASSERT_NE(xpkIndexRemove(xpk, 100), 0);
	ASSERT_EQ(xpkIndexUserData(xpk, 100), 0);
	ASSERT_NE(xpkIndexUserDataSet(xpk, 100, 123), 0);

	xpkClose(xpk);
}

int main(void) {
	xrtInit();

	printf("=================================================\n");
	printf("  xPack Ver7 - Index Operations Test\n");
	printf("=================================================\n\n");

	printf("[Index Basic Operations]\n");
	RUN_TEST(index_append_find);
	RUN_TEST(index_extract_file);
	RUN_TEST(index_extract_data);
	RUN_TEST(index_order_preservation);
	printf("\n");

	printf("[Index Duplicate Detection]\n");
	RUN_TEST(index_duplicate_detection);
	printf("\n");

	printf("[Index Update Operations]\n");
	RUN_TEST(index_update_file);
	RUN_TEST(index_update_data);
	printf("\n");

	printf("[Index Remove Operations]\n");
	RUN_TEST(index_remove);
	printf("\n");

	printf("[Index User Data]\n");
	RUN_TEST(index_user_data);
	printf("\n");

	printf("[Index Edge Cases]\n");
	RUN_TEST(index_negative_index);
	RUN_TEST(index_large_index);
	RUN_TEST(index_sparse_indices);
	RUN_TEST(index_operations_on_non_index);
	printf("\n");

	printf("=================================================\n");
	printf("  Results: %d passed, %d failed\n", tests_passed, tests_failed);
	printf("=================================================\n");

	return tests_failed > 0 ? 1 : 0;
}
