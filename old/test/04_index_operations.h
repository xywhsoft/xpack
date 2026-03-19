/*
 * xPack Ver7 - index_operations (04)
 */

#include "test_framework.h"

TEST(index_basic_access) {
	xpkObject xpk = xpkOpen("test_04_index_basic.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "First file", 10, 6);
	xpkAppendData(xpk, "Second file", 11, 6);
	xpkAppendData(xpk, "Third file", 10, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_04_index_basic.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 3);

	for (uint32_t i = 0; i < 3; i++) {
		uint32_t size = xpkInfoSize(xpk, i);
		ASSERT_GT(size, 0);
	}

	xpkClose(xpk);
}

TEST(index_empty_package) {
	xpkObject xpk = xpkOpen("test_04_index_empty.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkCount(xpk), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_04_index_empty.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 0);
	xpkClose(xpk);
}

TEST(index_large_count) {
	xpkObject xpk = xpkOpen("test_04_index_large.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 500; i++) {
		char data[128];
		sprintf(data, "File %d data", i);
		ASSERT_NE(xpkAppendData(xpk, data, (uint32_t)strlen(data), 6), UINT32_MAX);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_04_index_large.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 500);

	for (uint32_t i = 0; i < 500; i++) {
		ASSERT_GT(xpkInfoSize(xpk, i), 0);
	}

	xpkClose(xpk);
}

TEST(index_info_access) {
	xpkObject xpk = xpkOpen("test_04_index_info.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	char testData[512];
	memset(testData, 'A', sizeof(testData));

	xpkAppendData(xpk, testData, sizeof(testData), 9);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_04_index_info.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkInfoSize(xpk, 0), 512);
	ASSERT_GT(xpkInfoPacked(xpk, 0), 0);
	ASSERT_EQ(xpkInfoLevel(xpk, 0), 9);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_UNKNOWN);
	ASSERT_NE(xpkInfoHash(xpk, 0), 0);

	xpkClose(xpk);
}

TEST(index_type_set) {
	xpkObject xpk = xpkOpen("test_04_index_type.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Test data", 9, 6);
	ASSERT_EQ(xpkInfoTypeSet(xpk, 0, XPK_FTYPE_TEXT), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_04_index_type.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_TEXT);
	xpkClose(xpk);
}

TEST(index_multiple_types) {
	xpkObject xpk = xpkOpen("test_04_index_types.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Text file", 9, 6);
	xpkInfoTypeSet(xpk, 0, XPK_FTYPE_TEXT);

	xpkAppendData(xpk, "Binary data", 11, 6);
	xpkInfoTypeSet(xpk, 1, XPK_FTYPE_BINARY);

	xpkAppendData(xpk, "Code file", 9, 6);
	xpkInfoTypeSet(xpk, 2, XPK_FTYPE_TEXT);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_04_index_types.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 3);

	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_TEXT);
	ASSERT_EQ(xpkInfoType(xpk, 1), XPK_FTYPE_BINARY);
	ASSERT_EQ(xpkInfoType(xpk, 2), XPK_FTYPE_TEXT);

	xpkClose(xpk);
}

TEST(index_removal_updates_index) {
	xpkObject xpk = xpkOpen("test_04_index_removal.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "File 1", 6, 6);
	xpkAppendData(xpk, "File 2", 6, 6);
	xpkAppendData(xpk, "File 3", 6, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_04_index_removal.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 3);

	ASSERT_EQ(xpkRemove(xpk, 1), 0);
	ASSERT_EQ(xpkCount(xpk), 2);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_04_index_removal.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 2);
	xpkClose(xpk);
}

void register_04_index_operations_tests(void) {
	TEST_REGISTER(index_basic_access, CAT_INDEX, "Test basic index access");
	TEST_REGISTER(index_empty_package, CAT_INDEX, "Test index with empty package");
	TEST_REGISTER(index_large_count, CAT_INDEX, "Test index with large file count");
	TEST_REGISTER(index_info_access, CAT_INDEX, "Test info access via index");
	TEST_REGISTER(index_type_set, CAT_INDEX, "Test setting file type");
	TEST_REGISTER(index_multiple_types, CAT_INDEX, "Test multiple file types");
	TEST_REGISTER(index_removal_updates_index, CAT_INDEX, "Test removal updates index");
}
