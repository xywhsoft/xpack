/*
 * xPack Ver7 - 路径模式操作测试
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
#define ASSERT_STR_EQ(a, b) ASSERT(strcmp(a, b) == 0)

TEST(path_append_extract_data_win32) {
	xpkObject xpk = xpkOpen("test_05_win32_data.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	void* info = xpkPathAppendData(xpk, "data/test.bin", "BinaryData", 10, 6);
	ASSERT_NOT_NULL(info);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_05_win32_data.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t outSize = 0;
	void* data = xpkPathExtractData(xpk, "data/test.bin", &outSize);
	ASSERT_NOT_NULL(data);
	ASSERT_EQ(outSize, 10);
	ASSERT_EQ(memcmp(data, "BinaryData", 10), 0);
	xpkFree(data);

	xpkClose(xpk);
}

TEST(path_append_extract_data_linux) {
	xpkObject xpk = xpkOpen("test_05_linux_data.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_LINUX), 0);

	void* info = xpkPathAppendData(xpk, "config/settings.ini", "[Settings]", 10, 6);
	ASSERT_NOT_NULL(info);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_05_linux_data.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t outSize = 0;
	void* data = xpkPathExtractData(xpk, "config/settings.ini", &outSize);
	ASSERT_NOT_NULL(data);
	ASSERT_EQ(outSize, 10);
	ASSERT_EQ(memcmp(data, "[Settings]", 10), 0);
	xpkFree(data);

	xpkClose(xpk);
}

TEST(path_append_extract_file_win32) {
	xpkObject xpk = xpkOpen("test_05_win32_file.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	char srcPath[256];
	sprintf(srcPath, "test_src_05.txt");
	FILE* fp = fopen(srcPath, "wb");
	fwrite("File content for path test", 26, 1, fp);
	fclose(fp);

	void* info = xpkPathAppendFile(xpk, "docs/readme.txt", srcPath, 6);
	ASSERT_NOT_NULL(info);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_05_win32_file.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	char dstPath[256];
	sprintf(dstPath, "test_dst_05.txt");
	ASSERT_EQ(xpkPathExtractFile(xpk, "docs/readme.txt", dstPath), 0);

	fp = fopen(dstPath, "rb");
	char readData[64];
	size_t readSize = fread(readData, 1, sizeof(readData), fp);
	fclose(fp);

	ASSERT_EQ(readSize, 26);
	ASSERT_EQ(memcmp(readData, "File content for path test", 26), 0);

	xpkClose(xpk);

	xrtFileDelete(srcPath);
	xrtFileDelete(dstPath);
}

TEST(path_update_file) {
	xpkObject xpk = xpkOpen("test_05_update.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	xpkPathAppendData(xpk, "update/test.txt", "Original", 8, 6);

	char srcPath[256];
	sprintf(srcPath, "test_src_update.txt");
	FILE* fp = fopen(srcPath, "wb");
	fwrite("Updated content", 15, 1, fp);
	fclose(fp);

	ASSERT_EQ(xpkPathUpdateFile(xpk, "update/test.txt", srcPath, 6), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_05_update.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t outSize = 0;
	void* data = xpkPathExtractData(xpk, "update/test.txt", &outSize);
	ASSERT_NOT_NULL(data);
	ASSERT_EQ(outSize, 15);
	ASSERT_EQ(memcmp(data, "Updated content", 15), 0);
	xpkFree(data);

	xpkClose(xpk);

	xrtFileDelete(srcPath);
}

TEST(path_update_data) {
	xpkObject xpk = xpkOpen("test_05_update_data.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_LINUX), 0);

	xpkPathAppendData(xpk, "data/config.json", "{}", 2, 6);

	ASSERT_EQ(xpkPathUpdateData(xpk, "data/config.json", "{\"key\":\"value\"}", 16, 6), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_05_update_data.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t outSize = 0;
	void* data = xpkPathExtractData(xpk, "data/config.json", &outSize);
	ASSERT_NOT_NULL(data);
	ASSERT_EQ(outSize, 16);
	ASSERT_EQ(memcmp(data, "{\"key\":\"value\"}", 16), 0);
	xpkFree(data);

	xpkClose(xpk);
}

TEST(path_remove) {
	xpkObject xpk = xpkOpen("test_05_remove.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	xpkPathAppendData(xpk, "file1.txt", "Data 1", 6, 6);
	xpkPathAppendData(xpk, "file2.txt", "Data 2", 6, 6);
	xpkPathAppendData(xpk, "file3.txt", "Data 3", 6, 6);

	ASSERT_EQ(xpkPathRemove(xpk, "file2.txt"), 0);
	ASSERT_EQ(xpkCount(xpk), 2);
	ASSERT_EQ(xpkPathExists(xpk, "file2.txt"), 0);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_05_remove.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkCount(xpk), 2);
	ASSERT_EQ(xpkPathExists(xpk, "file1.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "file3.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "file2.txt"), 0);

	xpkClose(xpk);
}

TEST(path_get) {
	xpkObject xpk = xpkOpen("test_05_get.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	xpkPathAppendData(xpk, "path/to/file1.txt", "Data 1", 6, 6);
	xpkPathAppendData(xpk, "another/path.txt", "Data 2", 6, 6);
	xpkPathAppendData(xpk, "root.txt", "Data 3", 6, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_05_get.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	const char* path1 = xpkPathGet(xpk, 0);
	ASSERT_NOT_NULL(path1);
	ASSERT_STR_EQ(path1, "path/to/file1.txt");

	const char* path2 = xpkPathGet(xpk, 1);
	ASSERT_NOT_NULL(path2);
	ASSERT_STR_EQ(path2, "another/path.txt");

	const char* path3 = xpkPathGet(xpk, 2);
	ASSERT_NOT_NULL(path3);
	ASSERT_STR_EQ(path3, "root.txt");

	const char* path4 = xpkPathGet(xpk, 3);
	ASSERT_NULL(path4);

	xpkClose(xpk);
}

TEST(path_duplicate_detection) {
	xpkObject xpk = xpkOpen("test_05_duplicate.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	void* info1 = xpkPathAppendData(xpk, "test.txt", "First", 5, 6);
	ASSERT_NOT_NULL(info1);

	void* info2 = xpkPathAppendData(xpk, "test.txt", "Second", 6, 6);
	ASSERT_NULL(info2);
	ASSERT_NE(xpkLastError(), 0);

	ASSERT_EQ(xpkCount(xpk), 1);
	ASSERT_EQ(xpkPathExists(xpk, "test.txt"), 1);

	xpkClose(xpk);
}

TEST(path_special_characters) {
	xpkObject xpk = xpkOpen("test_05_special.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_LINUX), 0);

	xpkPathAppendData(xpk, "file-with-dash.txt", "Data 1", 6, 6);
	xpkPathAppendData(xpk, "file_with_underscore.txt", "Data 2", 6, 6);
	xpkPathAppendData(xpk, "file.with.dots.txt", "Data 3", 6, 6);
	xpkPathAppendData(xpk, "file with spaces.txt", "Data 4", 6, 6);

	ASSERT_EQ(xpkCount(xpk), 4);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_05_special.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkPathExists(xpk, "file-with-dash.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "file_with_underscore.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "file.with.dots.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "file with spaces.txt"), 1);

	xpkClose(xpk);
}

TEST(path_length_boundary) {
	xpkObject xpk = xpkOpen("test_05_length.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	char longPath[XPK_PATH_MAX + 10];
	memset(longPath, 'A', XPK_PATH_MAX - 1);
	longPath[XPK_PATH_MAX - 1] = '\0';
	strcpy(longPath + XPK_PATH_MAX - 5, ".txt");

	void* info = xpkPathAppendData(xpk, longPath, "Data", 4, 6);
	ASSERT_NOT_NULL(info);

	char tooLongPath[XPK_PATH_MAX + 10];
	memset(tooLongPath, 'B', XPK_PATH_MAX);
	tooLongPath[XPK_PATH_MAX] = '\0';
	strcat(tooLongPath, "/extra.txt");

	info = xpkPathAppendData(xpk, tooLongPath, "Data", 4, 6);
	ASSERT_NULL(info);
	ASSERT_NE(xpkLastError(), 0);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);
}

TEST(path_nested_directories) {
	xpkObject xpk = xpkOpen("test_05_nested.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_LINUX), 0);

	xpkPathAppendData(xpk, "root/level1/file1.txt", "Data 1", 6, 6);
	xpkPathAppendData(xpk, "root/level1/level2/file2.txt", "Data 2", 6, 6);
	xpkPathAppendData(xpk, "root/level1/level2/level3/file3.txt", "Data 3", 6, 6);
	xpkPathAppendData(xpk, "other/dir/file4.txt", "Data 4", 6, 6);

	ASSERT_EQ(xpkCount(xpk), 4);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_05_nested.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkPathExists(xpk, "root/level1/file1.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "root/level1/level2/file2.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "root/level1/level2/level3/file3.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "other/dir/file4.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "root/level1/level2/file3.txt"), 1);

	ASSERT_EQ(xpkPathExists(xpk, "root/level1/file2.txt"), 0);
	ASSERT_EQ(xpkPathExists(xpk, "root/level1/level2/file1.txt"), 0);

	xpkClose(xpk);
}

TEST(path_find_nonexistent) {
	xpkObject xpk = xpkOpen("test_05_find_none.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	xpkPathAppendData(xpk, "exists.txt", "Data", 4, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_05_find_none.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkPathExists(xpk, "exists.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "notexists.txt"), 0);
	ASSERT_EQ(xpkPathExists(xpk, ""), 0);
	ASSERT_EQ(xpkPathExists(xpk, "missing/file.txt"), 0);

	ASSERT_EQ(xpkPathFind(xpk, "exists.txt"), 0);
	ASSERT_EQ(xpkPathFind(xpk, "notexists.txt"), UINT32_MAX);

	uint32_t outSize = 0;
	void* data = xpkPathExtractData(xpk, "notexists.txt", &outSize);
	ASSERT_NULL(data);
	ASSERT_NE(xpkLastError(), 0);

	xpkClose(xpk);
}

TEST(path_operations_on_non_path_mode) {
	xpkObject xpk = xpkOpen("test_05_non_path.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkType(xpk), XPK_TYPE_CORE);

	ASSERT_EQ(xpkPathFind(xpk, "test.txt"), UINT32_MAX);
	ASSERT_EQ(xpkPathExists(xpk, "test.txt"), 0);
	ASSERT_NULL(xpkPathAppendData(xpk, "test.txt", "Data", 4, 6));
	ASSERT_NULL(xpkPathExtractData(xpk, "test.txt", NULL));
	ASSERT_NE(xpkPathUpdateData(xpk, "test.txt", "New", 3, 6), 0);
	ASSERT_NE(xpkPathRemove(xpk, "test.txt"), 0);
	ASSERT_NULL(xpkPathGet(xpk, 0));

	xpkClose(xpk);
}

TEST(path_backslash_conversion) {
	xpkObject xpk = xpkOpen("test_05_backslash.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	xpkPathAppendData(xpk, "path\\to\\file.txt", "Data", 4, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_05_backslash.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkPathExists(xpk, "path/to/file.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "path\\to\\file.txt"), 1);

	const char* path = xpkPathGet(xpk, 0);
	ASSERT_NOT_NULL(path);

	ASSERT_EQ(xpkPathExists(xpk, path), 1);

	xpkClose(xpk);
}

int main(void) {
	xrtInit();

	printf("=================================================\n");
	printf("  xPack Ver7 - Path Operations Test\n");
	printf("=================================================\n\n");

	printf("[Path Append/Extract - Win32]\n");
	RUN_TEST(path_append_extract_data_win32);
	RUN_TEST(path_append_extract_file_win32);
	printf("\n");

	printf("[Path Append/Extract - Linux]\n");
	RUN_TEST(path_append_extract_data_linux);
	printf("\n");

	printf("[Path Update Operations]\n");
	RUN_TEST(path_update_file);
	RUN_TEST(path_update_data);
	printf("\n");

	printf("[Path Remove Operations]\n");
	RUN_TEST(path_remove);
	printf("\n");

	printf("[Path Get Operations]\n");
	RUN_TEST(path_get);
	printf("\n");

	printf("[Path Special Cases]\n");
	RUN_TEST(path_duplicate_detection);
	RUN_TEST(path_special_characters);
	RUN_TEST(path_length_boundary);
	RUN_TEST(path_nested_directories);
	RUN_TEST(path_find_nonexistent);
	RUN_TEST(path_operations_on_non_path_mode);
	RUN_TEST(path_backslash_conversion);
	printf("\n");

	printf("=================================================\n");
	printf("  Results: %d passed, %d failed\n", tests_passed, tests_failed);
	printf("=================================================\n");

	return tests_failed > 0 ? 1 : 0;
}
