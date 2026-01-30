/*
 * xPack Ver7 - 批量操作测试
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
#define ASSERT_GT(a, b) ASSERT((a) > (b))

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

	char dstDir[256];
	sprintf(dstDir, "test_12_extract_dir");
	ASSERT_EQ(xpkExtractAll(xpk, dstDir), 0);

	ASSERT_EQ(xrtDirExists(dstDir), 1);

	xpkClose(xpk);

	xrtDirDelete(dstDir);
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

	char dstDir[256];
	sprintf(dstDir, "test_12_extract_path_dir");
	ASSERT_EQ(xpkExtractAll(xpk, dstDir), 0);

	ASSERT_EQ(xrtDirExists(dstDir), 1);
	ASSERT_EQ(xrtDirExists("test_12_extract_path_dir/dir1"), 1);
	ASSERT_EQ(xrtDirExists("test_12_extract_path_dir/dir2"), 1);

	xpkClose(xpk);

	xrtDirDelete(dstDir);
}

TEST(batch_append_dir_empty) {
	char testDir[256];
	sprintf(testDir, "test_12_empty_dir");
	xrtDirCreate(testDir);

	xpkObject xpk = xpkOpen("test_12_append_empty_dir.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppendDir(xpk, testDir, "*", 0), 0);
	ASSERT_EQ(xpkCount(xpk), 0);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xrtDirDelete(testDir);
}

TEST(batch_append_dir_with_files) {
	char testDir[256];
	sprintf(testDir, "test_12_files_dir");
	xrtDirCreate(testDir);

	char path1[256], path2[256], path3[256];
	sprintf(path1, "%s/file1.txt", testDir);
	sprintf(path2, "%s/file2.txt", testDir);
	sprintf(path3, "%s/file3.txt", testDir);

	FILE* fp;

	fp = fopen(path1, "wb");
	fwrite("Content 1", 9, 1, fp);
	fclose(fp);

	fp = fopen(path2, "wb");
	fwrite("Content 2", 9, 1, fp);
	fclose(fp);

	fp = fopen(path3, "wb");
	fwrite("Content 3", 9, 1, fp);
	fclose(fp);

	xpkObject xpk = xpkOpen("test_12_append_dir.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppendDir(xpk, testDir, "*", 6), 3);
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
	char testDir[256];
	sprintf(testDir, "test_12_pattern_dir");
	xrtDirCreate(testDir);

	char path1[256], path2[256], path3[256], path4[256];
	sprintf(path1, "%s/file1.txt", testDir);
	sprintf(path2, "%s/file2.txt", testDir);
	sprintf(path3, "%s/data.bin", testDir);
	sprintf(path4, "%s/info.log", testDir);

	FILE* fp;

	fp = fopen(path1, "wb");
	fwrite("Text 1", 6, 1, fp);
	fclose(fp);

	fp = fopen(path2, "wb");
	fwrite("Text 2", 6, 1, fp);
	fclose(fp);

	fp = fopen(path3, "wb");
	fwrite("Binary", 6, 1, fp);
	fclose(fp);

	fp = fopen(path4, "wb");
	fwrite("Log", 3, 1, fp);
	fclose(fp);

	xpkObject xpk = xpkOpen("test_12_append_pattern.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppendDir(xpk, testDir, "*.txt", 6), 2);
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
	char testDir[256];
	sprintf(testDir, "test_12_question_dir");
	xrtDirCreate(testDir);

	char path1[256], path2[256], path3[256];
	sprintf(path1, "%s/file1.txt", testDir);
	sprintf(path2, "%s/file2.txt", testDir);
	sprintf(path3, "%s/data.txt", testDir);

	FILE* fp;

	fp = fopen(path1, "wb");
	fwrite("File 1", 6, 1, fp);
	fclose(fp);

	fp = fopen(path2, "wb");
	fwrite("File 2", 6, 1, fp);
	fclose(fp);

	fp = fopen(path3, "wb");
	fwrite("Data", 4, 1, fp);
	fclose(fp);

	xpkObject xpk = xpkOpen("test_12_append_question.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppendDir(xpk, testDir, "file?.txt", 6), 2);
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
	char testDir[256];
	sprintf(testDir, "test_12_recursive_dir");
	xrtDirCreate(testDir);

	xrtDirCreate("test_12_recursive_dir/subdir1");
	xrtDirCreate("test_12_recursive_dir/subdir1/subsubdir");
	xrtDirCreate("test_12_recursive_dir/subdir2");

	char path1[256], path2[256], path3[256], path4[256];
	sprintf(path1, "%s/file1.txt", testDir);
	sprintf(path2, "%s/subdir1/file2.txt", testDir);
	sprintf(path3, "%s/subdir1/subsubdir/file3.txt", testDir);
	sprintf(path4, "%s/subdir2/file4.txt", testDir);

	FILE* fp;

	fp = fopen(path1, "wb");
	fwrite("Root", 4, 1, fp);
	fclose(fp);

	fp = fopen(path2, "wb");
	fwrite("Level 1", 7, 1, fp);
	fclose(fp);

	fp = fopen(path3, "wb");
	fwrite("Level 2", 7, 1, fp);
	fclose(fp);

	fp = fopen(path4, "wb");
	fwrite("Another", 7, 1, fp);
	fclose(fp);

	xpkObject xpk = xpkOpen("test_12_append_recursive.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);
	ASSERT_EQ(xpkAppendDir(xpk, testDir, "*", 6), 4);
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

TEST(batch_multiple_levels) {
	char testDir[256];
	sprintf(testDir, "test_12_levels_dir");
	xrtDirCreate(testDir);

	char path1[256], path2[256];
	sprintf(path1, "%s/level1.txt", testDir);
	sprintf(path2, "%s/level10.txt", testDir);

	FILE* fp;

	fp = fopen(path1, "wb");
	fwrite("Level 1", 7, 1, fp);
	fclose(fp);

	fp = fopen(path2, "wb");
	fwrite("Level 10", 8, 1, fp);
	fclose(fp);

	xpkObject xpk = xpkOpen("test_12_levels.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppendDir(xpk, testDir, "*.txt", 1), 2);
	ASSERT_EQ(xpkCount(xpk), 2);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_12_levels.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 2);

	ASSERT_EQ(xpkInfoLevel(xpk, 0), 1);
	ASSERT_EQ(xpkInfoLevel(xpk, 1), 1);

	xpkClose(xpk);

	xrtDirDelete(testDir);
}

TEST(batch_large_number_files) {
	char testDir[256];
	sprintf(testDir, "test_12_large_dir");
	xrtDirCreate(testDir);

	int fileCount = 100;

	for (int i = 0; i < fileCount; i++) {
		char path[256];
		sprintf(path, "%s/file%03d.txt", testDir, i);

		FILE* fp = fopen(path, "wb");
		char data[64];
		sprintf(data, "File content %d", i);
		fwrite(data, 1, strlen(data), fp);
		fclose(fp);
	}

	xpkObject xpk = xpkOpen("test_12_large.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppendDir(xpk, testDir, "*.txt", 6), fileCount);
	ASSERT_EQ(xpkCount(xpk), fileCount);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_12_large.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), fileCount);
	xpkClose(xpk);

	xrtDirDelete(testDir);
}

TEST(batch_mixed_sizes) {
	char testDir[256];
	sprintf(testDir, "test_12_mixed_dir");
	xrtDirCreate(testDir);

	char path1[256], path2[256], path3[256];
	sprintf(path1, "%s/small.txt", testDir);
	sprintf(path2, "%s/medium.txt", testDir);
	sprintf(path3, "%s/large.txt", testDir);

	FILE* fp;

	fp = fopen(path1, "wb");
	fwrite("Small", 5, 1, fp);
	fclose(fp);

	fp = fopen(path2, "wb");
	char medium[10240];
	memset(medium, 'M', sizeof(medium));
	fwrite(medium, 1, sizeof(medium), fp);
	fclose(fp);

	fp = fopen(path3, "wb");
	char large[102400];
	memset(large, 'L', sizeof(large));
	fwrite(large, 1, sizeof(large), fp);
	fclose(fp);

	xpkObject xpk = xpkOpen("test_12_mixed.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppendDir(xpk, testDir, "*.txt", 6), 3);
	ASSERT_EQ(xpkCount(xpk), 3);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_12_mixed.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 3);

	ASSERT_EQ(xpkInfoSize(xpk, 0), 5);
	ASSERT_EQ(xpkInfoSize(xpk, 1), 10240);
	ASSERT_EQ(xpkInfoSize(xpk, 2), 102400);

	xpkClose(xpk);

	xrtDirDelete(testDir);
}

TEST(batch_special_patterns) {
	char testDir[256];
	sprintf(testDir, "test_12_special_dir");
	xrtDirCreate(testDir);

	char path1[256], path2[256], path3[256];
	sprintf(path1, "%s/test.txt", testDir);
	sprintf(path2, "%s/test123.txt", testDir);
	sprintf(path3, "%s/data.txt", testDir);

	FILE* fp;

	fp = fopen(path1, "wb");
	fwrite("Test 1", 6, 1, fp);
	fclose(fp);

	fp = fopen(path2, "wb");
	fwrite("Test 123", 8, 1, fp);
	fclose(fp);

	fp = fopen(path3, "wb");
	fwrite("Data", 4, 1, fp);
	fclose(fp);

	xpkObject xpk = xpkOpen("test_12_special.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppendDir(xpk, testDir, "test*.txt", 6), 2);
	ASSERT_EQ(xpkCount(xpk), 2);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_12_special.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 2);
	xpkClose(xpk);

	xrtDirDelete(testDir);
}

int main(void) {
	xrtInit();

	printf("=================================================\n");
	printf("  xPack Ver7 - Batch Operations Test\n");
	printf("=================================================\n\n");

	printf("[Extract All Tests]\n");
	RUN_TEST(batch_extract_all_core);
	RUN_TEST(batch_extract_all_path);
	printf("\n");

	printf("[Append Dir Tests]\n");
	RUN_TEST(batch_append_dir_empty);
	RUN_TEST(batch_append_dir_with_files);
	printf("\n");

	printf("[Pattern Matching Tests]\n");
	RUN_TEST(batch_append_dir_pattern_txt);
	RUN_TEST(batch_append_dir_pattern_question);
	RUN_TEST(batch_special_patterns);
	printf("\n");

	printf("[Recursive Tests]\n");
	RUN_TEST(batch_append_dir_recursive);
	printf("\n");

	printf("[Compression Level Tests]\n");
	RUN_TEST(batch_multiple_levels);
	printf("\n");

	printf("[Large Scale Tests]\n");
	RUN_TEST(batch_large_number_files);
	RUN_TEST(batch_mixed_sizes);
	printf("\n");

	printf("=================================================\n");
	printf("  Results: %d passed, %d failed\n", tests_passed, tests_failed);
	printf("=================================================\n");

	return tests_failed > 0 ? 1 : 0;
}
