/*
 * xPack Ver7 - 错误处理测试
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
#define ASSERT_STR_CONTAINS(str, substr) ASSERT(strstr(str, substr) != NULL)

TEST(error_invalid_signature) {
	FILE* fp = fopen("test_11_invalid_sig.xpk", "wb");
	uint8_t fakeHeader[48] = { 0 };
	fwrite(fakeHeader, 1, 48, fp);
	fclose(fp);

	xpkObject xpk = xpkOpen("test_11_invalid_sig.xpk", 0, 1);
	ASSERT_NULL(xpk);
	ASSERT_NE(xpkLastError(), 0);
	ASSERT_STR_CONTAINS(xpkLastErrorMsg(), "signature");
	ASSERT_STR_CONTAINS(xpkLastErrorMsg(), "Invalid");

	xpkClose(xpk);

	xrtFileDelete("test_11_invalid_sig.xpk");
}

TEST(error_unsupported_version) {
	FILE* fp = fopen("test_11_bad_version.xpk", "wb");

	xpkHead fakeHead;
	memset(&fakeHead, 0, sizeof(xpkHead));
	fakeHead.signature = XPK_SIGNATURE;
	fakeHead.version = 99;
	fwrite(&fakeHead, 1, sizeof(xpkHead), fp);
	fclose(fp);

	xpkObject xpk = xpkOpen("test_11_bad_version.xpk", 0, 1);
	ASSERT_NULL(xpk);
	ASSERT_NE(xpkLastError(), 0);
	ASSERT_STR_CONTAINS(xpkLastErrorMsg(), "Version");
	ASSERT_STR_CONTAINS(xpkLastErrorMsg(), "not supported");

	xpkClose(xpk);

	xrtFileDelete("test_11_bad_version.xpk");
}

TEST(error_file_not_found) {
	xpkObject xpk = xpkOpen("nonexistent_file_12345.xpk", 0, 1);
	ASSERT_NULL(xpk);
	ASSERT_EQ(xpkLastError(), 1);

	ASSERT_STR_CONTAINS(xpkLastErrorMsg(), "File");
	ASSERT_STR_CONTAINS(xpkLastErrorMsg(), "not found");
}

TEST(error_readonly_write) {
	xpkObject xpk = xpkOpen("test_11_readonly_write.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Data", 4, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_11_readonly_write.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppendData(xpk, "New", 3, 6), UINT32_MAX);
	ASSERT_EQ(xpkLastError(), 10);
	ASSERT_STR_CONTAINS(xpkLastErrorMsg(), "readonly");

	ASSERT_EQ(xpkUpdateData(xpk, 0, "Updated", 7, 6), -1);
	ASSERT_EQ(xpkLastError(), 10);

	ASSERT_EQ(xpkRemove(xpk, 0), -1);
	ASSERT_EQ(xpkLastError(), 10);

	ASSERT_EQ(xpkInfoTypeSet(xpk, 0, XPK_FTYPE_TEXT), -1);
	ASSERT_EQ(xpkLastError(), 10);

	xpkClose(xpk);
}

TEST(error_invalid_position) {
	xpkObject xpk = xpkOpen("test_11_invalid_pos.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Data", 4, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_11_invalid_pos.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_NULL(xpkInfo(xpk, 1));
	ASSERT_EQ(xpkLastError(), 6);

	ASSERT_NULL(xpkExtractData(xpk, 1, NULL));
	ASSERT_EQ(xpkLastError(), 6);

	ASSERT_EQ(xpkUpdateData(xpk, 1, "New", 3, 6), -1);
	ASSERT_EQ(xpkLastError(), 6);

	ASSERT_EQ(xpkRemove(xpk, 1), -1);
	ASSERT_EQ(xpkLastError(), 6);

	ASSERT_EQ(xpkInfoSize(xpk, 1), 0);
	ASSERT_EQ(xpkInfoPacked(xpk, 1), 0);
	ASSERT_EQ(xpkInfoHash(xpk, 1), 0);
	ASSERT_EQ(xpkInfoLevel(xpk, 1), -1);
	ASSERT_EQ(xpkInfoType(xpk, 1), -1);

	xpkClose(xpk);
}

TEST(error_null_object) {
	ASSERT_NULL(xpkInfo(NULL, 0));
	ASSERT_EQ(xpkCount(NULL), 0);
	ASSERT_EQ(xpkType(NULL), -1);
	ASSERT_EQ(xpkDiscCode(NULL), 0);
	ASSERT_EQ(xpkSave(NULL), -1);

	ASSERT_EQ(xpkAppendData(NULL, "Data", 4, 6), UINT32_MAX);
	ASSERT_EQ(xpkAppendFile(NULL, "file.txt", 6), UINT32_MAX);
	ASSERT_NULL(xpkExtractData(NULL, 0, NULL));
	ASSERT_EQ(xpkExtractFile(NULL, 0, "out.txt"), -1);
	ASSERT_EQ(xpkUpdateData(NULL, 0, "New", 3, 6), -1);
	ASSERT_EQ(xpkUpdateFile(NULL, 0, "file.txt", 6), -1);
	ASSERT_EQ(xpkRemove(NULL, 0), -1);
	ASSERT_EQ(xpkInfoSize(NULL, 0), 0);
	ASSERT_EQ(xpkInfoPacked(NULL, 0), 0);
	ASSERT_EQ(xpkInfoHash(NULL, 0), 0);
	ASSERT_EQ(xpkInfoLevel(NULL, 0), -1);
	ASSERT_EQ(xpkInfoType(NULL, 0), -1);
	ASSERT_EQ(xpkInfoTypeSet(NULL, 0, XPK_FTYPE_TEXT), -1);
}

TEST(error_pack_type_change_after_files) {
	xpkObject xpk = xpkOpen("test_11_type_change.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Data", 4, 6);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), -1);
	ASSERT_EQ(xpkLastError(), 11);
	ASSERT_STR_CONTAINS(xpkLastErrorMsg(), "type");

	ASSERT_EQ(xpkType(xpk), XPK_TYPE_CORE);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);
}

TEST(error_index_not_found) {
	xpkObject xpk = xpkOpen("test_11_index_not_found.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

	xpkIndexAppendData(xpk, 100, "Data", 4, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_11_index_not_found.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkIndexFind(xpk, 999), UINT32_MAX);
	ASSERT_NULL(xpkIndexExtractData(xpk, 999, NULL));
	ASSERT_EQ(xpkIndexUpdateData(xpk, 999, "New", 3, 6), -1);
	ASSERT_EQ(xpkIndexRemove(xpk, 999), -1);
	ASSERT_EQ(xpkIndexUserData(xpk, 999), 0);
	ASSERT_EQ(xpkIndexUserDataSet(xpk, 999, 123), -1);

	xpkClose(xpk);
}

TEST(error_index_duplicate) {
	xpkObject xpk = xpkOpen("test_11_index_dup.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

	xpkIndexAppendData(xpk, 100, "First", 5, 6);
	ASSERT_NULL(xpkIndexAppendData(xpk, 100, "Second", 6, 6));
	ASSERT_NE(xpkLastError(), 0);

	ASSERT_EQ(xpkCount(xpk), 1);
	ASSERT_NE(xpkIndexFind(xpk, 100), UINT32_MAX);

	xpkClose(xpk);
}

TEST(error_path_not_found) {
	xpkObject xpk = xpkOpen("test_11_path_not_found.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	xpkPathAppendData(xpk, "exists.txt", "Data", 4, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_11_path_not_found.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkPathExists(xpk, "exists.txt"), 1);
	ASSERT_EQ(xpkPathExists(xpk, "missing.txt"), 0);
	ASSERT_EQ(xpkPathFind(xpk, "missing.txt"), UINT32_MAX);
	ASSERT_NULL(xpkPathExtractData(xpk, "missing.txt", NULL));
	ASSERT_EQ(xpkPathUpdateData(xpk, "missing.txt", "New", 3, 6), -1);
	ASSERT_EQ(xpkPathRemove(xpk, "missing.txt"), -1);

	xpkClose(xpk);
}

TEST(error_path_duplicate) {
	xpkObject xpk = xpkOpen("test_11_path_dup.xpk", 0, 0);
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

TEST(error_path_too_long) {
	xpkObject xpk = xpkOpen("test_11_path_long.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	char longPath[XPK_PATH_MAX + 10];
	memset(longPath, 'A', XPK_PATH_MAX + 5);
	longPath[XPK_PATH_MAX + 5] = '\0';

	void* info = xpkPathAppendData(xpk, longPath, "Data", 4, 6);
	ASSERT_NULL(info);
	ASSERT_NE(xpkLastError(), 0);

	ASSERT_STR_CONTAINS(xpkLastErrorMsg(), "Path");
	ASSERT_STR_CONTAINS(xpkLastErrorMsg(), "long");

	xpkClose(xpk);
}

TEST(error_null_parameters) {
	xpkObject xpk = xpkOpen("test_11_null_params.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppendData(xpk, NULL, 0, 6), 0);
	ASSERT_EQ(xpkAppendData(xpk, "Data", 0, 6), 0);
	ASSERT_EQ(xpkAppendData(xpk, NULL, 0, 6), 0);

	ASSERT_NULL(xpkExtractData(xpk, 0, NULL));
	ASSERT_EQ(xpkExtractFile(xpk, 0, NULL), -1);
	ASSERT_EQ(xpkAppendFile(xpk, NULL, 6), UINT32_MAX);
	ASSERT_EQ(xpkUpdateData(xpk, 0, NULL, 0, 6), 0);
	ASSERT_EQ(xpkUpdateFile(xpk, 0, NULL, 6), -1);

	ASSERT_NULL(xpkIndexAppendData(xpk, 100, NULL, 0, 6));
	ASSERT_NULL(xpkIndexAppendData(NULL, 100, "Data", 4, 6));
	ASSERT_NULL(xpkIndexExtractData(NULL, 100, NULL));
	ASSERT_NULL(xpkIndexExtractData(xpk, 100, NULL));

	ASSERT_NULL(xpkPathAppendData(xpk, NULL, "Data", 4, 6));
	ASSERT_NULL(xpkPathAppendData(xpk, "path.txt", NULL, 0, 6));
	ASSERT_NULL(xpkPathAppendData(xpk, NULL, NULL, 0, 6));
	ASSERT_NULL(xpkPathExtractData(xpk, NULL, NULL));
	ASSERT_NULL(xpkPathExtractData(xpk, "path.txt", NULL));
	ASSERT_EQ(xpkPathExtractFile(xpk, NULL, "out.txt"), -1);
	ASSERT_EQ(xpkPathExtractFile(xpk, "path.txt", NULL), -1);

	xpkClose(xpk);
}

TEST(error_wrong_pack_type) {
	xpkObject xpk = xpkOpen("test_11_wrong_type.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkType(xpk), XPK_TYPE_CORE);

	ASSERT_EQ(xpkIndexFind(xpk, 100), UINT32_MAX);
	ASSERT_NULL(xpkIndexAppendData(xpk, 100, "Data", 4, 6));
	ASSERT_NULL(xpkIndexExtractData(xpk, 100, NULL));
	ASSERT_EQ(xpkIndexUpdateData(xpk, 100, "New", 3, 6), -1);
	ASSERT_EQ(xpkIndexRemove(xpk, 100), -1);

	ASSERT_EQ(xpkPathFind(xpk, "test.txt"), UINT32_MAX);
	ASSERT_EQ(xpkPathExists(xpk, "test.txt"), 0);
	ASSERT_NULL(xpkPathAppendData(xpk, "test.txt", "Data", 4, 6));
	ASSERT_NULL(xpkPathExtractData(xpk, "test.txt", NULL));
	ASSERT_EQ(xpkPathUpdateData(xpk, "test.txt", "New", 3, 6), -1);
	ASSERT_EQ(xpkPathRemove(xpk, "test.txt"), -1);
	ASSERT_NULL(xpkPathGet(xpk, 0));

	xpkClose(xpk);
}

TEST(error_message_clearing) {
	xpkObject xpk = xpkOpen("test_11_error_msg.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_NULL(xpkInfo(xpk, 0));
	ASSERT_NE(xpkLastError(), 0);
	ASSERT_NE(xpkLastErrorMsg(), NULL);

	xpkAppendData(xpk, "Data", 4, 6);
	ASSERT_EQ(xpkLastError(), 0);

	xpkClose(xpk);
}

TEST(error_callback_registration) {
	xpkObject xpk = xpkOpen("test_11_callback.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	int callbackCalled = 0;

	void errorCallback(int code, const char* msg) {
		callbackCalled = 1;
	}

	xpkOnError(xpk, errorCallback);

	xpkAppendData(xpk, "Data", 4, 6);
	ASSERT_EQ(xpkSave(xpk), 0);

	ASSERT_EQ(callbackCalled, 0);

	ASSERT_NULL(xpkInfo(xpk, 1));
	ASSERT_EQ(callbackCalled, 1);

	xpkClose(xpk);
}

int main(void) {
	xrtInit();

	printf("=================================================\n");
	printf("  xPack Ver7 - Error Handling Test\n");
	printf("=================================================\n\n");

	printf("[File Format Errors]\n");
	RUN_TEST(error_invalid_signature);
	RUN_TEST(error_unsupported_version);
	RUN_TEST(error_file_not_found);
	printf("\n");

	printf("[Mode Errors]\n");
	RUN_TEST(error_readonly_write);
	RUN_TEST(error_pack_type_change_after_files);
	RUN_TEST(error_wrong_pack_type);
	printf("\n");

	printf("[Position/Index Errors]\n");
	RUN_TEST(error_invalid_position);
	RUN_TEST(error_index_not_found);
	RUN_TEST(error_index_duplicate);
	printf("\n");

	printf("[Path Errors]\n");
	RUN_TEST(error_path_not_found);
	RUN_TEST(error_path_duplicate);
	RUN_TEST(error_path_too_long);
	printf("\n");

	printf("[Parameter Errors]\n");
	RUN_TEST(error_null_object);
	RUN_TEST(error_null_parameters);
	printf("\n");

	printf("[Error Message Tests]\n");
	RUN_TEST(error_message_clearing);
	RUN_TEST(error_callback_registration);
	printf("\n");

	printf("=================================================\n");
	printf("  Results: %d passed, %d failed\n", tests_passed, tests_failed);
	printf("=================================================\n");

	return tests_failed > 0 ? 1 : 0;
}
