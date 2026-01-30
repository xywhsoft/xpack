/*
 * xPack Ver7 - 路径大小写敏感测试
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

TEST(win32_case_insensitive) {
	xpkObject xpk = xpkOpen("test_06_win32_case.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	xpkPathAppendData(xpk, "file.txt", "Data 1", 6, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_06_win32_case.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkPathExists(xpk, "file.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "FILE.TXT"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "File.Txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "fIlE.tXt"), 1);

	ASSERT_NE(xpkPathFind(xpk, "file.txt"), UINT32_MAX);
	ASSERT_NE(xpkPathFind(xpk, "FILE.TXT"), UINT32_MAX);
	ASSERT_NE(xpkPathFind(xpk, "File.Txt"), UINT32_MAX);

	ASSERT_EQ(xpkCount(xpk), 1);

	xpkClose(xpk);
}

TEST(win32_duplicate_detection) {
	xpkObject xpk = xpkOpen("test_06_win32_dup.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	void* info1 = xpkPathAppendData(xpk, "file.txt", "Data 1", 6, 6);
	ASSERT_NOT_NULL(info1);

	void* info2 = xpkPathAppendData(xpk, "FILE.TXT", "Data 2", 6, 6);
	ASSERT_NULL(info2);
	ASSERT_NE(xpkLastError(), 0);

	void* info3 = xpkPathAppendData(xpk, "File.Txt", "Data 3", 6, 6);
	ASSERT_NULL(info3);
	ASSERT_NE(xpkLastError(), 0);

	ASSERT_EQ(xpkCount(xpk), 1);
	ASSERT_EQ(xpkPathExists(xpk, "file.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "FILE.TXT"), 1);

	xpkClose(xpk);
}

TEST(win32_mixed_case_paths) {
	xpkObject xpk = xpkOpen("test_06_win32_mixed.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	xpkPathAppendData(xpk, "Dir1/File1.txt", "Data 1", 6, 6);
	xpkPathAppendData(xpk, "dir1/file2.txt", "Data 2", 6, 6);
	xpkPathAppendData(xpk, "DIR1/FILE3.TXT", "Data 3", 6, 6);
	xpkPathAppendData(xpk, "DIR2/file4.txt", "Data 4", 6, 6);

	ASSERT_EQ(xpkCount(xpk), 4);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_06_win32_mixed.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkPathExists(xpk, "dir1/file1.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "DIR1/FILE1.TXT"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "dir2/file4.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "DIR2/FILE4.TXT"), 1);

	ASSERT_EQ(xpkCount(xpk), 4);

	xpkClose(xpk);
}

TEST(win32_slash_conversion) {
	xpkObject xpk = xpkOpen("test_06_win32_slash.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	xpkPathAppendData(xpk, "dir\\file.txt", "Data 1", 6, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_06_win32_slash.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkPathExists(xpk, "dir/file.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "dir\\file.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "DIR/FILE.TXT"), 1);

	ASSERT_EQ(xpkCount(xpk), 1);

	xpkClose(xpk);
}

TEST(linux_case_sensitive) {
	xpkObject xpk = xpkOpen("test_06_linux_case.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_LINUX), 0);

	xpkPathAppendData(xpk, "file.txt", "Data 1", 6, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_06_linux_case.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkPathExists(xpk, "file.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "FILE.TXT"), 0);
	ASSERT_EQ(xpkPathExists(xpk, "File.Txt"), 0);
	ASSERT_EQ(xpkPathExists(xpk, "fIlE.tXt"), 0);

	ASSERT_NE(xpkPathFind(xpk, "file.txt"), UINT32_MAX);
	ASSERT_EQ(xpkPathFind(xpk, "FILE.TXT"), UINT32_MAX);
	ASSERT_EQ(xpkPathFind(xpk, "File.Txt"), UINT32_MAX);

	ASSERT_EQ(xpkCount(xpk), 1);

	xpkClose(xpk);
}

TEST(linux_multiple_cases) {
	xpkObject xpk = xpkOpen("test_06_linux_multi.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_LINUX), 0);

	void* info1 = xpkPathAppendData(xpk, "file.txt", "Data 1", 6, 6);
	ASSERT_NOT_NULL(info1);

	void* info2 = xpkPathAppendData(xpk, "FILE.TXT", "Data 2", 6, 6);
	ASSERT_NOT_NULL(info2);

	void* info3 = xpkPathAppendData(xpk, "File.Txt", "Data 3", 6, 6);
	ASSERT_NOT_NULL(info3);

	ASSERT_EQ(xpkCount(xpk), 3);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_06_linux_multi.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkPathExists(xpk, "file.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "FILE.TXT"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "File.Txt"), 1);

	ASSERT_NE(xpkPathFind(xpk, "file.txt"), UINT32_MAX);
	ASSERT_NE(xpkPathFind(xpk, "FILE.TXT"), UINT32_MAX);
	ASSERT_NE(xpkPathFind(xpk, "File.Txt"), UINT32_MAX);

	ASSERT_EQ(xpkCount(xpk), 3);

	xpkClose(xpk);
}

TEST(linux_case_directories) {
	xpkObject xpk = xpkOpen("test_06_linux_dir.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_LINUX), 0);

	xpkPathAppendData(xpk, "Dir1/file.txt", "Data 1", 6, 6);
	xpkPathAppendData(xpk, "dir1/file.txt", "Data 2", 6, 6);
	xpkPathAppendData(xpk, "DIR1/file.txt", "Data 3", 6, 6);

	ASSERT_EQ(xpkCount(xpk), 3);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_06_linux_dir.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkPathExists(xpk, "Dir1/file.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "dir1/file.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "DIR1/file.txt"), 1);

	ASSERT_EQ(xpkPathExists(xpk, "dir1/File.TXT"), 0);
	ASSERT_EQ(xpkPathExists(xpk, "DIR1/FILE.TXT"), 0);

	ASSERT_EQ(xpkCount(xpk), 3);

	xpkClose(xpk);
}

TEST(win32_vs_linux) {
	xpkObject win32_xpk = xpkOpen("test_06_win32_only.xpk", 0, 0);
	ASSERT_NOT_NULL(win32_xpk);

	ASSERT_EQ(xpkTypeSet(win32_xpk, XPK_TYPE_WIN32), 0);

	xpkPathAppendData(win32_xpk, "File.txt", "Data", 4, 6);
	ASSERT_EQ(xpkCount(win32_xpk), 1);

	ASSERT_EQ(xpkSave(win32_xpk), 0);
	xpkClose(win32_xpk);

	xpkObject linux_xpk = xpkOpen("test_06_linux_only.xpk", 0, 0);
	ASSERT_NOT_NULL(linux_xpk);

	ASSERT_EQ(xpkTypeSet(linux_xpk, XPK_TYPE_LINUX), 0);

	xpkPathAppendData(linux_xpk, "File.txt", "Data", 4, 6);
	ASSERT_EQ(xpkCount(linux_xpk), 1);

	ASSERT_EQ(xpkSave(linux_xpk), 0);
	xpkClose(linux_xpk);

	win32_xpk = xpkOpen("test_06_win32_only.xpk", 0, 1);
	ASSERT_NOT_NULL(win32_xpk);

	ASSERT_EQ(xpkPathExists(win32_xpk, "file.txt"), 1);
	ASSERT_EQ(xpkPathExists(win32_xpk, "FILE.TXT"), 1);
	xpkClose(win32_xpk);

	linux_xpk = xpkOpen("test_06_linux_only.xpk", 0, 1);
	ASSERT_NOT_NULL(linux_xpk);

	ASSERT_EQ(xpkPathExists(linux_xpk, "File.txt"), 1);
	ASSERT_EQ(xpkPathExists(linux_xpk, "file.txt"), 0);
	ASSERT_EQ(xpkPathExists(linux_xpk, "FILE.TXT"), 0);
	xpkClose(linux_xpk);
}

TEST(win32_update_case) {
	xpkObject xpk = xpkOpen("test_06_win32_update.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	xpkPathAppendData(xpk, "file.txt", "Original", 8, 6);

	ASSERT_EQ(xpkPathUpdateData(xpk, "FILE.TXT", "Updated", 7, 6), 0);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_06_win32_update.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkCount(xpk), 1);

	uint32_t outSize = 0;
	void* data = xpkPathExtractData(xpk, "file.txt", &outSize);
	ASSERT_NOT_NULL(data);
	ASSERT_EQ(outSize, 7);
	ASSERT_EQ(memcmp(data, "Updated", 7), 0);
	xpkFree(data);

	ASSERT_EQ(xpkPathExists(xpk, "FILE.TXT"), 1);

	xpkClose(xpk);
}

TEST(linux_update_case) {
	xpkObject xpk = xpkOpen("test_06_linux_update.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_LINUX), 0);

	xpkPathAppendData(xpk, "file.txt", "Original", 8, 6);
	xpkPathAppendData(xpk, "FILE.TXT", "Different", 9, 6);

	ASSERT_EQ(xpkCount(xpk), 2);

	ASSERT_EQ(xpkPathUpdateData(xpk, "file.txt", "Updated", 7, 6), 0);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_06_linux_update.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkCount(xpk), 2);

	uint32_t outSize = 0;
	void* data = xpkPathExtractData(xpk, "file.txt", &outSize);
	ASSERT_NOT_NULL(data);
	ASSERT_EQ(outSize, 7);
	ASSERT_EQ(memcmp(data, "Updated", 7), 0);
	xpkFree(data);

	data = xpkPathExtractData(xpk, "FILE.TXT", &outSize);
	ASSERT_NOT_NULL(data);
	ASSERT_EQ(outSize, 9);
	ASSERT_EQ(memcmp(data, "Different", 9), 0);
	xpkFree(data);

	xpkClose(xpk);
}

TEST(win32_remove_case) {
	xpkObject xpk = xpkOpen("test_06_win32_remove.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	xpkPathAppendData(xpk, "file.txt", "Data 1", 6, 6);
	xpkPathAppendData(xpk, "other.txt", "Data 2", 6, 6);

	ASSERT_EQ(xpkPathRemove(xpk, "FILE.TXT"), 0);
	ASSERT_EQ(xpkCount(xpk), 1);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_06_win32_remove.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkCount(xpk), 1);
	ASSERT_EQ(xpkPathExists(xpk, "file.txt"), 0);
	ASSERT_EQ(xpkPathExists(xpk, "FILE.TXT"), 0);
	ASSERT_EQ(xpkPathExists(xpk, "other.txt"), 1);

	xpkClose(xpk);
}

TEST(linux_remove_case) {
	xpkObject xpk = xpkOpen("test_06_linux_remove.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_LINUX), 0);

	xpkPathAppendData(xpk, "file.txt", "Data 1", 6, 6);
	xpkPathAppendData(xpk, "FILE.TXT", "Data 2", 6, 6);

	ASSERT_EQ(xpkPathRemove(xpk, "file.txt"), 0);
	ASSERT_EQ(xpkCount(xpk), 1);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_06_linux_remove.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkCount(xpk), 1);
	ASSERT_EQ(xpkPathExists(xpk, "file.txt"), 0);
	ASSERT_EQ(xpkPathExists(xpk, "FILE.TXT"), 1);

	xpkClose(xpk);
}

TEST(win32_extract_case) {
	xpkObject xpk = xpkOpen("test_06_win32_extract.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	xpkPathAppendData(xpk, "file.txt", "Original content", 17, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_06_win32_extract.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t outSize = 0;
	void* data = xpkPathExtractData(xpk, "FILE.TXT", &outSize);
	ASSERT_NOT_NULL(data);
	ASSERT_EQ(outSize, 17);
	ASSERT_EQ(memcmp(data, "Original content", 17), 0);
	xpkFree(data);

	data = xpkPathExtractData(xpk, "File.Txt", &outSize);
	ASSERT_NOT_NULL(data);
	ASSERT_EQ(outSize, 17);
	ASSERT_EQ(memcmp(data, "Original content", 17), 0);
	xpkFree(data);

	xpkClose(xpk);
}

TEST(linux_extract_case) {
	xpkObject xpk = xpkOpen("test_06_linux_extract.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_LINUX), 0);

	xpkPathAppendData(xpk, "file.txt", "Original content", 17, 6);
	xpkPathAppendData(xpk, "FILE.TXT", "Different content", 17, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_06_linux_extract.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t outSize = 0;
	void* data = xpkPathExtractData(xpk, "file.txt", &outSize);
	ASSERT_NOT_NULL(data);
	ASSERT_EQ(outSize, 17);
	ASSERT_EQ(memcmp(data, "Original content", 17), 0);
	xpkFree(data);

	data = xpkPathExtractData(xpk, "FILE.TXT", &outSize);
	ASSERT_NOT_NULL(data);
	ASSERT_EQ(outSize, 17);
	ASSERT_EQ(memcmp(data, "Different content", 17), 0);
	xpkFree(data);

	data = xpkPathExtractData(xpk, "File.Txt", &outSize);
	ASSERT_NULL(data);

	xpkClose(xpk);
}

TEST(win32_special_chars_case) {
	xpkObject xpk = xpkOpen("test_06_win32_special.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	xpkPathAppendData(xpk, "File_With_Dash.txt", "Data 1", 6, 6);
	xpkPathAppendData(xpk, "file_with_dash.txt", "Data 2", 6, 6);

	ASSERT_EQ(xpkCount(xpk), 2);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_06_win32_special.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkPathExists(xpk, "file_with_dash.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "FILE_WITH_DASH.TXT"), 1);

	ASSERT_EQ(xpkCount(xpk), 2);

	xpkClose(xpk);
}

int main(void) {
	xrtInit();

	printf("=================================================\n");
	printf("  xPack Ver7 - Path Case Sensitivity Test\n");
	printf("=================================================\n\n");

	printf("[Win32 Case Insensitive Tests]\n");
	RUN_TEST(win32_case_insensitive);
	RUN_TEST(win32_duplicate_detection);
	RUN_TEST(win32_mixed_case_paths);
	RUN_TEST(win32_slash_conversion);
	printf("\n");

	printf("[Linux Case Sensitive Tests]\n");
	RUN_TEST(linux_case_sensitive);
	RUN_TEST(linux_multiple_cases);
	RUN_TEST(linux_case_directories);
	printf("\n");

	printf("[Win32 vs Linux Comparison]\n");
	RUN_TEST(win32_vs_linux);
	printf("\n");

	printf("[Update Operations Case Tests]\n");
	RUN_TEST(win32_update_case);
	RUN_TEST(linux_update_case);
	printf("\n");

	printf("[Remove Operations Case Tests]\n");
	RUN_TEST(win32_remove_case);
	RUN_TEST(linux_remove_case);
	printf("\n");

	printf("[Extract Operations Case Tests]\n");
	RUN_TEST(win32_extract_case);
	RUN_TEST(linux_extract_case);
	printf("\n");

	printf("[Special Characters Case Tests]\n");
	RUN_TEST(win32_special_chars_case);
	printf("\n");

	printf("=================================================\n");
	printf("  Results: %d passed, %d failed\n", tests_passed, tests_failed);
	printf("=================================================\n");

	return tests_failed > 0 ? 1 : 0;
}
