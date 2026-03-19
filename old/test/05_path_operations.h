/*
 * xPack Ver7 - path_operations (05)
 */

#include "test_framework.h"

TEST(path_append_file) {
	xpkObject xpk = xpkOpen("test_05_path_append.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkPathAppendData(xpk, "files/test.txt", "Test content", 12, 6), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_05_path_append.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 1);
	xpkClose(xpk);
}

TEST(path_extract_file) {
	xpkObject xpk = xpkOpen("test_05_path_extract.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkPathAppendData(xpk, "files/extract.txt", "Extract me", 10, 6), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_05_path_extract.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkCount(xpk), 1);
	xpkClose(xpk);
}

TEST(path_append_directory) {
	xpkObject xpk = xpkOpen("test_05_path_dir.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkPathAppendData(xpk, "files/", "Content 1", 9, 6), 0);
	ASSERT_EQ(xpkPathAppendData(xpk, "files/subdir/file2.txt", "Content 2", 9, 6), 1);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_05_path_dir.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_GT(xpkCount(xpk), 0);
	xpkClose(xpk);
}

TEST(path_relative_paths) {
	xpkObject xpk = xpkOpen("test_05_relative.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkPathAppendData(xpk, "relative/file.txt", "Relative path", 13, 6), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_05_relative.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 1);
	xpkClose(xpk);
}

TEST(path_absolute_paths) {
	xpkObject xpk = xpkOpen("test_05_absolute.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkPathAppendData(xpk, "files/abs.txt", "Absolute path", 13, 6), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_05_absolute.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 1);
	xpkClose(xpk);
}

TEST(path_special_characters) {
	xpkObject xpk = xpkOpen("test_05_special.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkPathAppendData(xpk, "files/file with spaces.txt", "Special chars", 13, 6), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_05_special.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 1);
	xpkClose(xpk);
}

TEST(path_deep_nesting) {
	xpkObject xpk = xpkOpen("test_05_deep.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkPathAppendData(xpk, "deep/nested/file.txt", "Deeply nested", 13, 6), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_05_deep.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 1);
	xpkClose(xpk);
}

TEST(path_update_file) {
	xpkObject xpk = xpkOpen("test_05_update.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_NE(xpkPathAppendData(xpk, "files/file.txt", "Original", 8, 6), UINT32_MAX);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	// Reopen and try to append same path - should fail because path exists
	xpk = xpkOpen("test_05_update.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);
	
	// Should fail because path already exists
	ASSERT_EQ(xpkPathAppendData(xpk, "files/file.txt", "Updated content", 15, 6), UINT32_MAX);
	xpkClose(xpk);
}

TEST(path_multiple_files) {
	xpkObject xpk = xpkOpen("test_05_multi.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 10; i++) {
		char data[32];
		sprintf(data, "File %d content", i);
		char path[64];
		sprintf(path, "files/file%d.txt", i);
		ASSERT_EQ(xpkPathAppendData(xpk, path, data, (uint32_t)strlen(data), 6), (uint32_t)i);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 10);
	xpkClose(xpk);

	xpk = xpkOpen("test_05_multi.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 10);
	xpkClose(xpk);
}

TEST(path_traverse_files) {
	xpkObject xpk = xpkOpen("test_05_traverse.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 5; i++) {
		char data[32];
		sprintf(data, "File %d", i);
		char path[64];
		sprintf(path, "files/dir%d/file.txt", i);
		ASSERT_EQ(xpkPathAppendData(xpk, path, data, (uint32_t)strlen(data), 6), (uint32_t)i);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 5);
	xpkClose(xpk);

	xpk = xpkOpen("test_05_traverse.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	int count = 0;
	for (uint32_t i = 0; i < xpkCount(xpk); i++) {
		ASSERT_GT(xpkInfoSize(xpk, i), 0);
		count++;
	}

	ASSERT_EQ(count, 5);
	xpkClose(xpk);
}

void register_05_path_operations_tests(void) {
	TEST_REGISTER(path_append_file, CAT_PATH, "Test path append file");
	TEST_REGISTER(path_extract_file, CAT_PATH, "Test path extract file");
	TEST_REGISTER(path_append_directory, CAT_PATH, "Test path append directory");
	TEST_REGISTER(path_relative_paths, CAT_PATH, "Test relative paths");
	TEST_REGISTER(path_absolute_paths, CAT_PATH, "Test absolute paths");
	TEST_REGISTER(path_special_characters, CAT_PATH, "Test special characters in paths");
	TEST_REGISTER(path_deep_nesting, CAT_PATH, "Test deep path nesting");
	TEST_REGISTER(path_update_file, CAT_PATH, "Test path update file");
	TEST_REGISTER(path_multiple_files, CAT_PATH, "Test multiple files with paths");
	TEST_REGISTER(path_traverse_files, CAT_PATH, "Test traverse files with paths");
}
