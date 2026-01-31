/*
 * xPack Ver7 - batch_operations (12)
 */

#include "test_framework.h"

TEST(batch_append_many) {
	xpkObject xpk = xpkOpen("test_12_batch_append.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 100; i++) {
		char data[256];
		sprintf(data, "Batch file %d", i);
		ASSERT_NE(xpkAppendData(xpk, data, (uint32_t)strlen(data), 6), UINT32_MAX);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 100);
	xpkClose(xpk);

	xpk = xpkOpen("test_12_batch_append.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 100);

	for (uint32_t i = 0; i < 100; i++) {
		ASSERT_GT(xpkInfoSize(xpk, i), 0);
	}

	xpkClose(xpk);
}

TEST(batch_remove_many) {
	xpkObject xpk = xpkOpen("test_12_batch_remove.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 50; i++) {
		char data[128];
		sprintf(data, "File %d", i);
		ASSERT_NE(xpkAppendData(xpk, data, (uint32_t)strlen(data), 6), UINT32_MAX);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 50);
	xpkClose(xpk);

	xpk = xpkOpen("test_12_batch_remove.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 50);

	for (uint32_t i = 0; i < 25; i++) {
		ASSERT_EQ(xpkRemove(xpk, 0), 0);
	}

	ASSERT_EQ(xpkCount(xpk), 25);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_12_batch_remove.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 25);
	xpkClose(xpk);
}

TEST(batch_extract_all) {
	xpkObject xpk = xpkOpen("test_12_batch_extract.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	char testData[10][256];
	for (int i = 0; i < 10; i++) {
		memset(testData[i], 'A' + i, 256);
		sprintf(testData[i], "Test data %d - ", i);
		ASSERT_NE(xpkAppendData(xpk, testData[i], 256, 6), UINT32_MAX);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 10);
	xpkClose(xpk);

	xpk = xpkOpen("test_12_batch_extract.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	for (uint32_t i = 0; i < 10; i++) {
		uint32_t outSize = 0;
		void* extracted = xpkExtractData(xpk, i, &outSize);
		ASSERT_NOT_NULL(extracted);
		ASSERT_EQ(outSize, 256);
		ASSERT_EQ(memcmp(extracted, testData[i], 256), 0);
		xpkFree(extracted);
	}

	xpkClose(xpk);
}

TEST(batch_update_many) {
	xpkObject xpk = xpkOpen("test_12_batch_update.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 20; i++) {
		char data[128];
		sprintf(data, "Original %d", i);
		ASSERT_NE(xpkAppendData(xpk, data, (uint32_t)strlen(data), 6), UINT32_MAX);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 20);
	xpkClose(xpk);

	xpk = xpkOpen("test_12_batch_update.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	for (uint32_t i = 0; i < 20; i++) {
		char newData[128];
		sprintf(newData, "Updated %d", i);
		ASSERT_EQ(xpkUpdateData(xpk, i, newData, (uint32_t)strlen(newData), 6), 0);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_12_batch_update.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	for (uint32_t i = 0; i < 20; i++) {
		uint32_t outSize = 0;
		void* extracted = xpkExtractData(xpk, i, &outSize);
		ASSERT_NOT_NULL(extracted);
		ASSERT_GT(outSize, 0);
		xpkFree(extracted);
	}

	xpkClose(xpk);
}

TEST(batch_mixed_operations) {
	xpkObject xpk = xpkOpen("test_12_batch_mixed.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 30; i++) {
		char data[256];
		sprintf(data, "File %d", i);
		ASSERT_NE(xpkAppendData(xpk, data, (uint32_t)strlen(data), 6), UINT32_MAX);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 30);
	xpkClose(xpk);

	xpk = xpkOpen("test_12_batch_mixed.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 10; i++) {
		ASSERT_EQ(xpkRemove(xpk, 0), 0);
	}

	ASSERT_EQ(xpkCount(xpk), 20);

	for (uint32_t i = 0; i < 10; i++) {
		char newData[256];
		sprintf(newData, "Updated file %d", i);
		ASSERT_EQ(xpkUpdateData(xpk, i, newData, (uint32_t)strlen(newData), 6), 0);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_12_batch_mixed.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 20);
	xpkClose(xpk);
}

TEST(batch_large_volume) {
	xpkObject xpk = xpkOpen("test_12_batch_large.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 200; i++) {
		char data[1024];
		memset(data, 'A' + (i % 26), 1024);
		ASSERT_NE(xpkAppendData(xpk, data, 1024, 6), UINT32_MAX);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 200);
	xpkClose(xpk);

	xpk = xpkOpen("test_12_batch_large.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 200);

	for (uint32_t i = 0; i < 200; i++) {
		ASSERT_GT(xpkInfoSize(xpk, i), 0);
	}

	xpkClose(xpk);
}

TEST(batch_extract_all_core) {
	xpkObject xpk = xpkOpen("test_12_extract_all.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Data 1", 6, 6);
	xpkAppendData(xpk, "Data 2", 6, 6);
	xpkAppendData(xpk, "Data 3", 6, 6);

	ASSERT_EQ(xpkCount(xpk), 3);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_12_extract_all.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkExtractAll(xpk, "test_12_extract_dir"), 0);

	ASSERT_EQ(xrtDirExists("test_12_extract_dir"), 1);

	xpkClose(xpk);

	xrtDirDelete("test_12_extract_dir");
}

TEST(batch_extract_all_path) {
	xpkObject xpk = xpkOpen("test_12_extract_all_path.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	xpkPathAppendData(xpk, "dir1/file1.txt", "Content 1", 9, 6);
	xpkPathAppendData(xpk, "dir1/file2.txt", "Content 2", 9, 6);
	xpkPathAppendData(xpk, "dir2/file3.txt", "Content 3", 9, 6);

	ASSERT_EQ(xpkCount(xpk), 3);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_12_extract_all_path.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkExtractAll(xpk, "test_12_extract_path_dir"), 0);

	ASSERT_EQ(xrtDirExists("test_12_extract_path_dir"), 1);
	ASSERT_EQ(xrtDirExists("test_12_extract_path_dir/dir1"), 1);
	ASSERT_EQ(xrtDirExists("test_12_extract_path_dir/dir2"), 1);

	xpkClose(xpk);

	xrtDirDelete("test_12_extract_path_dir");
}

TEST(batch_append_dir_with_files) {
	char testDir[] = "test_12_files_dir";
	xrtDirCreate(testDir);

	FILE* fp;

	fp = fopen("test_12_files_dir/file1.txt", "wb");
	fwrite("Content 1", 9, 1, fp);
	fclose(fp);

	fp = fopen("test_12_files_dir/file2.txt", "wb");
	fwrite("Content 2", 9, 1, fp);
	fclose(fp);

	fp = fopen("test_12_files_dir/file3.txt", "wb");
	fwrite("Content 3", 9, 1, fp);
	fclose(fp);

	xpkObject xpk = xpkOpen("test_12_append_dir.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppendDir(xpk, testDir, "*", 6, 0), 3);
	ASSERT_EQ(xpkCount(xpk), 3);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_12_append_dir.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 3);
	xpkClose(xpk);

	xrtDirDelete(testDir);
}

TEST(batch_append_dir_pattern_txt) {
	char testDir[] = "test_12_pattern_dir";
	xrtDirCreate(testDir);

	FILE* fp;

	fp = fopen("test_12_pattern_dir/file1.txt", "wb");
	fwrite("Text 1", 6, 1, fp);
	fclose(fp);

	fp = fopen("test_12_pattern_dir/file2.txt", "wb");
	fwrite("Text 2", 6, 1, fp);
	fclose(fp);

	fp = fopen("test_12_pattern_dir/data.bin", "wb");
	fwrite("Binary", 6, 1, fp);
	fclose(fp);

	fp = fopen("test_12_pattern_dir/info.log", "wb");
	fwrite("Log", 3, 1, fp);
	fclose(fp);

	xpkObject xpk = xpkOpen("test_12_append_pattern.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppendDir(xpk, testDir, "*.txt", 6, 0), 2);
	ASSERT_EQ(xpkCount(xpk), 2);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_12_append_pattern.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 2);

	uint32_t outSize = 0;
	void* data1 = xpkExtractData(xpk, 0, &outSize);
	ASSERT_NOT_NULL(data1);
	ASSERT_EQ(outSize, 6);
	xpkFree(data1);

	void* data2 = xpkExtractData(xpk, 1, &outSize);
	ASSERT_NOT_NULL(data2);
	ASSERT_EQ(outSize, 6);
	xpkFree(data2);

	xpkClose(xpk);

	xrtDirDelete(testDir);
}

TEST(batch_append_dir_pattern_question) {
	char testDir[] = "test_12_question_dir";
	xrtDirCreate(testDir);

	FILE* fp;

	fp = fopen("test_12_question_dir/file1.txt", "wb");
	fwrite("File 1", 6, 1, fp);
	fclose(fp);

	fp = fopen("test_12_question_dir/file2.txt", "wb");
	fwrite("File 2", 6, 1, fp);
	fclose(fp);

	fp = fopen("test_12_question_dir/data.txt", "wb");
	fwrite("Data", 4, 1, fp);
	fclose(fp);

	xpkObject xpk = xpkOpen("test_12_append_question.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppendDir(xpk, testDir, "file?.txt", 6, 0), 2);
	ASSERT_EQ(xpkCount(xpk), 2);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_12_append_question.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 2);
	xpkClose(xpk);

	xrtDirDelete(testDir);
}

TEST(batch_append_dir_recursive) {
	char testDir[] = "test_12_recursive_dir";
	xrtDirCreate(testDir);
	xrtDirCreate("test_12_recursive_dir/subdir1");
	xrtDirCreate("test_12_recursive_dir/subdir1/subsubdir");
	xrtDirCreate("test_12_recursive_dir/subdir2");

	FILE* fp;

	fp = fopen("test_12_recursive_dir/file1.txt", "wb");
	fwrite("Root", 4, 1, fp);
	fclose(fp);

	fp = fopen("test_12_recursive_dir/subdir1/file2.txt", "wb");
	fwrite("Level 1", 7, 1, fp);
	fclose(fp);

	fp = fopen("test_12_recursive_dir/subdir1/subsubdir/file3.txt", "wb");
	fwrite("Level 2", 7, 1, fp);
	fclose(fp);

	fp = fopen("test_12_recursive_dir/subdir2/file4.txt", "wb");
	fwrite("Another", 7, 1, fp);
	fclose(fp);

	xpkObject xpk = xpkOpen("test_12_append_recursive.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);
	ASSERT_EQ(xpkAppendDir(xpk, testDir, "*", 6, 1), 4);
	ASSERT_EQ(xpkCount(xpk), 4);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_12_append_recursive.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 4);

	ASSERT_EQ(xpkPathExists(xpk, "file1.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "subdir1/file2.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "subdir1/subsubdir/file3.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "subdir2/file4.txt"), 1);

	xpkClose(xpk);

	xrtDirDelete(testDir);
}

void register_12_batch_operations_tests(void) {
	TEST_REGISTER(batch_append_many, CAT_BATCH, "Test batch append many files");
	TEST_REGISTER(batch_remove_many, CAT_BATCH, "Test batch remove many files");
	TEST_REGISTER(batch_extract_all, CAT_BATCH, "Test batch extract all files");
	TEST_REGISTER(batch_update_many, CAT_BATCH, "Test batch update many files");
	TEST_REGISTER(batch_mixed_operations, CAT_BATCH, "Test batch mixed operations");
	TEST_REGISTER(batch_large_volume, CAT_BATCH, "Test batch large volume operations");
	TEST_REGISTER(batch_extract_all_core, CAT_BATCH, "Test extract all in core mode");
	TEST_REGISTER(batch_extract_all_path, CAT_BATCH, "Test extract all in path mode");
	TEST_REGISTER(batch_append_dir_with_files, CAT_BATCH, "Test append directory with files");
	TEST_REGISTER(batch_append_dir_pattern_txt, CAT_BATCH, "Test append directory with *.txt pattern");
	TEST_REGISTER(batch_append_dir_pattern_question, CAT_BATCH, "Test append directory with ? pattern");
	TEST_REGISTER(batch_append_dir_recursive, CAT_BATCH, "Test append directory recursively");
}
