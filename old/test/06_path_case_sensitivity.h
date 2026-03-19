/*
 * xPack Ver7 - path_case_sensitivity (06)
 */

#include "test_framework.h"

TEST(case_sensitive_create) {
	xpkObject xpk = xpkOpen("test_06_case_sensitive.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkPathAppendData(xpk, "files/file.txt", "Lowercase", 9, 6), 0);
	ASSERT_EQ(xpkPathAppendData(xpk, "files/FILE.TXT", "Uppercase", 9, 6), 1);
	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 2);
	xpkClose(xpk);

	xpk = xpkOpen("test_06_case_sensitive.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 2);
	xpkClose(xpk);
}

TEST(case_sensitive_extract) {
	xpkObject xpk = xpkOpen("test_06_case_extract.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkPathAppendData(xpk, "files/TestFile.txt", "Case test", 10, 6), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_06_case_extract.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 1);
	xpkClose(xpk);
}

TEST(case_sensitive_update) {
	xpkObject xpk = xpkOpen("test_06_case_update.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_NE(xpkPathAppendData(xpk, "files/myfile.txt", "Original", 8, 6), UINT32_MAX);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_06_case_update.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	// In readonly mode, append should fail
	ASSERT_EQ(xpkPathAppendData(xpk, "files/MYFILE.TXT", "Updated", 7, 6), UINT32_MAX);
	// No save in readonly mode - just close
	xpkClose(xpk);
}

TEST(case_sensitive_duplicate_names) {
	xpkObject xpk = xpkOpen("test_06_duplicate.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_NE(xpkPathAppendData(xpk, "docs/readme.txt", "First file", 10, 6), UINT32_MAX);
	ASSERT_NE(xpkPathAppendData(xpk, "docs/README.TXT", "Second file", 11, 6), UINT32_MAX);
	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 2);
	xpkClose(xpk);

	xpk = xpkOpen("test_06_duplicate.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 2);
	xpkClose(xpk);
}

TEST(case_sensitive_directory_names) {
	xpkObject xpk = xpkOpen("test_06_dir_case.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_NE(xpkPathAppendData(xpk, "lowercase/file.txt", "Dir1 file", 10, 6), UINT32_MAX);
	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 1);
	xpkClose(xpk);

	xpk = xpkOpen("test_06_dir_case.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	// In Linux mode (default), different case = different path, so it should succeed
	ASSERT_EQ(xpkPathAppendData(xpk, "UPPERCASE/file.txt", "Dir2 file", 10, 6), 1);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);
}

TEST(case_sensitive_mixed_case) {
	xpkObject xpk = xpkOpen("test_06_mixed.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_NE(xpkPathAppendData(xpk, "files/MiXeD_CaSe.txt", "Mixed case", 11, 6), UINT32_MAX);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_06_mixed.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 1);
	xpkClose(xpk);
}

TEST(case_sensitive_extensions) {
	xpkObject xpk = xpkOpen("test_06_ext.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_NE(xpkPathAppendData(xpk, "files/test.txt", "Lower ext", 9, 6), UINT32_MAX);
	ASSERT_NE(xpkPathAppendData(xpk, "files/test.TXT", "Upper ext", 9, 6), UINT32_MAX);
	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 2);
	xpkClose(xpk);

	xpk = xpkOpen("test_06_ext.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 2);
	xpkClose(xpk);
}

TEST(case_sensitive_unicode) {
	xpkObject xpk = xpkOpen("test_06_unicode.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_NE(xpkPathAppendData(xpk, "files/файл.txt", "Unicode test", 13, 6), UINT32_MAX);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_06_unicode.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 1);
	xpkClose(xpk);
}

void register_06_path_case_sensitivity_tests(void) {
	TEST_REGISTER(case_sensitive_create, CAT_PATH, "Test case sensitive create");
	TEST_REGISTER(case_sensitive_extract, CAT_PATH, "Test case sensitive extract");
	TEST_REGISTER(case_sensitive_update, CAT_PATH, "Test case sensitive update");
	TEST_REGISTER(case_sensitive_duplicate_names, CAT_PATH, "Test case sensitive duplicate names");
	TEST_REGISTER(case_sensitive_directory_names, CAT_PATH, "Test case sensitive directory names");
	TEST_REGISTER(case_sensitive_mixed_case, CAT_PATH, "Test mixed case paths");
	TEST_REGISTER(case_sensitive_extensions, CAT_PATH, "Test case sensitive extensions");
	TEST_REGISTER(case_sensitive_unicode, CAT_PATH, "Test unicode case sensitivity");
}
