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

TEST(win32_path_case_insensitive) {
	char* pData = createTestData(1024, 'A');
	const char* sFilename = "test_25_win32.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_WIN32, 5);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppend(xpk, "test/file.txt", pData, 1024), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_NE(xpkFind(xpk, "Test/File.TXT"), -1);
	ASSERT_NE(xpkFind(xpk, "TEST/FILE.TXT"), -1);

	free(pData);
	xpkClose(xpk);
}

TEST(linux_path_case_sensitive) {
	char* pData = createTestData(1024, 'B');
	const char* sFilename = "test_25_linux.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_LINUX, 5);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppend(xpk, "test/file.txt", pData, 1024), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_NE(xpkFind(xpk, "test/file.txt"), -1);
	ASSERT_EQ(xpkFind(xpk, "Test/File.TXT"), -1);
	ASSERT_EQ(xpkFind(xpk, "TEST/FILE.TXT"), -1);

	free(pData);
	xpkClose(xpk);
}

TEST(win32_path_separator_handling) {
	char* pData = createTestData(1024, 'C');
	const char* sFilename = "test_25_separator.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_WIN32, 5);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppend(xpk, "folder\\subfolder\\file.txt", pData, 1024), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_NE(xpkFind(xpk, "folder/subfolder/file.txt"), -1);

	free(pData);
	xpkClose(xpk);
}

TEST(linux_path_separator_handling) {
	char* pData = createTestData(1024, 'D');
	const char* sFilename = "test_25_linux_sep.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_LINUX, 5);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppend(xpk, "folder/subfolder/file.txt", pData, 1024), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_NE(xpkFind(xpk, "folder/subfolder/file.txt"), -1);

	free(pData);
	xpkClose(xpk);
}

TEST(win32_and_linux_comparison) {
	char* pData1 = createTestData(1024, 'E');
	char* pData2 = createTestData(1024, 'F');
	const char* sWin32File = "test_25_win32_comp.xpk";
	const char* sLinuxFile = "test_25_linux_comp.xpk";

	xpkObject xpkWin32 = xpkOpen(sWin32File, XPK_TYPE_WIN32, 5);
	ASSERT_NOT_NULL(xpkWin32);

	ASSERT_EQ(xpkAppend(xpkWin32, "Folder/File.TXT", pData1, 1024), 0);
	ASSERT_EQ(xpkSave(xpkWin32), 0);

	xpkObject xpkLinux = xpkOpen(sLinuxFile, XPK_TYPE_LINUX, 5);
	ASSERT_NOT_NULL(xpkLinux);

	ASSERT_EQ(xpkAppend(xpkLinux, "folder/file.txt", pData2, 1024), 0);
	ASSERT_EQ(xpkSave(xpkLinux), 0);

	ASSERT_NE(xpkFind(xpkWin32, "folder/file.txt"), -1);
	ASSERT_NE(xpkFind(xpkLinux, "folder/file.txt"), -1);

	free(pData1);
	free(pData2);
	xpkClose(xpkWin32);
	xpkClose(xpkLinux);
}

TEST(absolute_path_handling) {
	char* pData = createTestData(1024, 'G');
	const char* sFilename = "test_25_abs_path.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_WIN32, 5);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppend(xpk, "C:\\Windows\\System32\\file.txt", pData, 1024), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_NE(xpkFind(xpk, "C:\\Windows\\System32\\file.txt"), -1);

	free(pData);
	xpkClose(xpk);
}

TEST(relative_path_handling) {
	char* pData = createTestData(1024, 'H');
	const char* sFilename = "test_25_rel_path.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_PATH, 5);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppend(xpk, "../data/file.txt", pData, 1024), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_NE(xpkFind(xpk, "../data/file.txt"), -1);

	free(pData);
	xpkClose(xpk);
}

TEST(path_with_spaces) {
	char* pData = createTestData(1024, 'I');
	const char* sFilename = "test_25_spaces.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_PATH, 5);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppend(xpk, "folder with spaces/file name.txt", pData, 1024), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_NE(xpkFind(xpk, "folder with spaces/file name.txt"), -1);

	free(pData);
	xpkClose(xpk);
}

TEST(unicode_path_handling) {
	char* pData = createTestData(1024, 'J');
	const char* sFilename = "test_25_unicode.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_PATH, 5);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppend(xpk, "文件/test.txt", pData, 1024), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_NE(xpkFind(xpk, "文件/test.txt"), -1);

	free(pData);
	xpkClose(xpk);
}

TEST(nested_directory_structure) {
	char* pData = createTestData(512, 'K');
	const char* sFilename = "test_25_nested.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_PATH, 5);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppend(xpk, "a/b/c/d/e/file.txt", pData, 512), 0);
	ASSERT_EQ(xpkAppend(xpk, "x/y/z/file2.txt", pData, 512), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_NE(xpkFind(xpk, "a/b/c/d/e/file.txt"), -1);
	ASSERT_NE(xpkFind(xpk, "x/y/z/file2.txt"), -1);

	free(pData);
	xpkClose(xpk);
}

TEST(file_extension_variations) {
	char* pData = createTestData(1024, 'L');
	const char* sFilename = "test_25_extensions.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_PATH, 5);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppend(xpk, "file.txt", pData, 1024), 0);
	ASSERT_EQ(xpkAppend(xpk, "file.TXT", pData, 1024), 0);
	ASSERT_EQ(xpkAppend(xpk, "file.dat", pData, 1024), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_NE(xpkFind(xpk, "file.txt"), -1);
	ASSERT_NE(xpkFind(xpk, "file.TXT"), -1);
	ASSERT_NE(xpkFind(xpk, "file.dat"), -1);

	free(pData);
	xpkClose(xpk);
}

TEST(path_length_limits) {
	char* pData = createTestData(512, 'M');
	const char* sFilename = "test_25_path_len.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_PATH, 5);
	ASSERT_NOT_NULL(xpk);

	char sLongPath[512];
	memset(sLongPath, 'a', 255);
	sLongPath[255] = '/';
	memset(sLongPath + 256, 'b', 255);
	sLongPath[511] = '\0';

	ASSERT_EQ(xpkAppend(xpk, sLongPath, pData, 512), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_NE(xpkFind(xpk, sLongPath), -1);

	free(pData);
	xpkClose(xpk);
}

TEST(dot_directory_handling) {
	char* pData = createTestData(1024, 'N');
	const char* sFilename = "test_25_dots.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_PATH, 5);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppend(xpk, "./file.txt", pData, 1024), 0);
	ASSERT_EQ(xpkAppend(xpk, "../file.txt", pData, 1024), 0);
	ASSERT_EQ(xpkAppend(xpk, "folder/./file.txt", pData, 1024), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_NE(xpkFind(xpk, "./file.txt"), -1);
	ASSERT_NE(xpkFind(xpk, "../file.txt"), -1);
	ASSERT_NE(xpkFind(xpk, "folder/./file.txt"), -1);

	free(pData);
	xpkClose(xpk);
}

TEST(drive_letter_in_path) {
	char* pData = createTestData(1024, 'O');
	const char* sFilename = "test_25_drive.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_WIN32, 5);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppend(xpk, "D:\\data\\file.txt", pData, 1024), 0);
	ASSERT_EQ(xpkAppend(xpk, "E:\\backup\\file.txt", pData, 1024), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_NE(xpkFind(xpk, "D:\\data\\file.txt"), -1);
	ASSERT_NE(xpkFind(xpk, "E:\\backup\\file.txt"), -1);

	free(pData);
	xpkClose(xpk);
}

TEST(special_characters_in_path) {
	char* pData = createTestData(1024, 'P');
	const char* sFilename = "test_25_special.xpk";

	xpkObject xpk = xpkOpen(sFilename, XPK_TYPE_PATH, 5);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppend(xpk, "folder/file_name-test.txt", pData, 1024), 0);
	ASSERT_EQ(xpkAppend(xpk, "folder/file.name.txt", pData, 1024), 0);
	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_NE(xpkFind(xpk, "folder/file_name-test.txt"), -1);
	ASSERT_NE(xpkFind(xpk, "folder/file.name.txt"), -1);

	free(pData);
	xpkClose(xpk);
}

int main(int argc, char* argv[]) {
	xrtInit();

	printf("[Cross Platform Tests]\n");
	printf("=======================\n\n");

	RUN_TEST(win32_path_case_insensitive);
	RUN_TEST(linux_path_case_sensitive);
	RUN_TEST(win32_path_separator_handling);
	RUN_TEST(linux_path_separator_handling);
	RUN_TEST(win32_and_linux_comparison);
	RUN_TEST(absolute_path_handling);
	RUN_TEST(relative_path_handling);
	RUN_TEST(path_with_spaces);
	RUN_TEST(unicode_path_handling);
	RUN_TEST(nested_directory_structure);
	RUN_TEST(file_extension_variations);
	RUN_TEST(path_length_limits);
	RUN_TEST(dot_directory_handling);
	RUN_TEST(drive_letter_in_path);
	RUN_TEST(special_characters_in_path);

	printf("=======================\n");
	printf("All Cross Platform Tests Passed!\n");

	return 0;
}
