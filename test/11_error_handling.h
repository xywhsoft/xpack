/*
 * xPack Ver7 - error_handling (11)
 */

#include "test_framework.h"

TEST(error_null_pointer) {
	xpkObject xpk = xpkOpen(NULL, 0, 0);
	ASSERT_NULL(xpk);

	int error = xpkLastError();
	ASSERT_NE(error, 0);
}

TEST(error_invalid_path) {
	xpkObject xpk = xpkOpen("", 0, 0);
	ASSERT_NULL(xpk);

	int error = xpkLastError();
	ASSERT_NE(error, 0);
}

TEST(error_readonly_write) {
	xpkObject xpk = xpkOpen("test_11_ro_write.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_11_ro_write.xpk", 1, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t pos = xpkAppendData(xpk, "Test", 4, 6);
	ASSERT_EQ(pos, UINT32_MAX);

	int error = xpkLastError();
	ASSERT_NE(error, 0);

	xpkClose(xpk);
}

TEST(error_out_of_range) {
	xpkObject xpk = xpkOpen("test_11_oor.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Test", 4, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_11_oor.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	uint32_t outSize = 0;
	void* data = xpkExtractData(xpk, 999, &outSize);
	ASSERT_NULL(data);

	int error = xpkLastError();
	ASSERT_NE(error, 0);

	xpkClose(xpk);
}

TEST(error_invalid_level) {
	xpkObject xpk = xpkOpen("test_11_invalid_level.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int level = -10; level < 0; level++) {
		uint32_t pos = xpkAppendData(xpk, "Test", 4, level);
		ASSERT_NE(pos, UINT32_MAX);
	}

	for (int level = 13; level < 20; level++) {
		uint32_t pos = xpkAppendData(xpk, "Test", 4, level);
		ASSERT_NE(pos, UINT32_MAX);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);
}

TEST(error_empty_data) {
	xpkObject xpk = xpkOpen("test_11_empty_data.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	uint32_t pos = xpkAppendData(xpk, NULL, 0, 6);
	ASSERT_NE(pos, UINT32_MAX);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_11_empty_data.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 1);
	xpkClose(xpk);
}

TEST(error_remove_nonexistent) {
	xpkObject xpk = xpkOpen("test_11_remove_none.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Test", 4, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_11_remove_none.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	int ret = xpkRemove(xpk, 999);
	ASSERT_NE(ret, 0);

	xpkClose(xpk);
}

TEST(error_update_nonexistent) {
	xpkObject xpk = xpkOpen("test_11_update_none.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Test", 4, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_11_update_none.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	int ret = xpkUpdateData(xpk, 999, "New", 3, 6);
	ASSERT_NE(ret, 0);

	xpkClose(xpk);
}

TEST(error_corrupted_package) {
	xpkObject xpk = xpkOpen("test_11_corrupt.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Test", 4, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	FILE* f = fopen("test_11_corrupt.xpk", "rb+");
	if (f) {
		fseek(f, 0, SEEK_SET);
		char garbage[1024];
		memset(garbage, 'X', sizeof(garbage));
		fwrite(garbage, 1, sizeof(garbage), f);
		fclose(f);
	}

	xpk = xpkOpen("test_11_corrupt.xpk", 0, 1);
	ASSERT_NULL(xpk);

	int error = xpkLastError();
	ASSERT_NE(error, 0);
}

TEST(error_invalid_signature) {
	FILE* f = fopen("test_11_bad_sig.xpk", "wb");
	if (f) {
		fwrite("BAD_SIGNATURE", 1, 12, f);
		fclose(f);
	}

	xpkObject xpk = xpkOpen("test_11_bad_sig.xpk", 0, 1);
	ASSERT_NULL(xpk);

	int error = xpkLastError();
	ASSERT_NE(error, 0);
}

TEST(error_unsupported_version) {
	FILE* f = fopen("test_11_bad_ver.xpk", "wb");
	if (f) {
		fwrite("XPKv99", 1, 6, f);
		fclose(f);
	}

	xpkObject xpk = xpkOpen("test_11_bad_ver.xpk", 0, 1);
	ASSERT_NULL(xpk);

	int error = xpkLastError();
	ASSERT_NE(error, 0);
}

TEST(error_file_not_found) {
	xpkObject xpk = xpkOpen("nonexistent_file_12345.xpk", 0, 1);
	ASSERT_NULL(xpk);

	int error = xpkLastError();
	ASSERT_NE(error, 0);
}

TEST(error_invalid_position) {
	xpkObject xpk = xpkOpen("test_11_bad_pos.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Test", 4, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_11_bad_pos.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkInfoSize(xpk, 9999), -1);
	ASSERT_EQ(xpkInfoPacked(xpk, 9999), 0);

	int error = xpkLastError();
	ASSERT_NE(error, 0);

	xpkClose(xpk);
}

TEST(error_null_object) {
	ASSERT_EQ(xpkSave(NULL), -1);
	ASSERT_EQ(xpkCount(NULL), -1);
	ASSERT_EQ(xpkRebuild(NULL), -1);
}

TEST(error_pack_type_change_after_files) {
	xpkObject xpk = xpkOpen("test_11_type_change.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	xpkAppendData(xpk, "Test", 4, 6);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_LINUX), -1);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);
}

TEST(error_index_not_found) {
	xpkObject xpk = xpkOpen("test_11_index_not_found.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

	xpkIndexAppendData(xpk, 100, "Test", 4, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_11_index_not_found.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkInfoSize(xpk, 200), -1);

	xpkClose(xpk);
}

TEST(error_index_duplicate) {
	xpkObject xpk = xpkOpen("test_11_index_dup.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

	ASSERT_NOT_NULL(xpkIndexAppendData(xpk, 100, "Test1", 5, 6));
	ASSERT_NULL(xpkIndexAppendData(xpk, 101, "Test2", 5, 6));

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);
}

TEST(error_path_duplicate) {
	xpkObject xpk = xpkOpen("test_11_path_dup.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	ASSERT_NOT_NULL(xpkPathAppendData(xpk, "files/test.txt", "Test1", 5, 6));
	ASSERT_NULL(xpkPathAppendData(xpk, "files/test.txt", "Test2", 5, 6));

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);
}

TEST(error_path_too_long) {
	xpkObject xpk = xpkOpen("test_11_long_path.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	char longPath[512];
	memset(longPath, 'a', 506);
	longPath[506] = '\0';
	strcat(longPath, ".txt");

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	void* ret = xpkPathAppendData(xpk, longPath, "Test", 4, 6);
	ASSERT_NULL(ret);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);
}

TEST(error_null_parameters) {
	xpkObject xpk = xpkOpen("test_11_null_params.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkAppendData(xpk, NULL, 10, 6), UINT32_MAX);

	ASSERT_EQ(xpkUpdateData(xpk, 0, NULL, 10, 6), -1);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);
}

TEST(error_wrong_pack_type) {
	xpkObject xpk = xpkOpen("test_11_wrong_type.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	ASSERT_NULL(xpkIndexAppendData(xpk, 100, "Test", 4, 6));

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);
}

TEST(error_message_clearing) {
	xpkObject xpk = xpkOpen("test_11_msg_clear.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Test", 4, 6);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_11_msg_clear.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	int error1 = xpkLastError();
	ASSERT_NE(error1, 0);

	int error2 = xpkLastError();
	ASSERT_EQ(error2, 0);

	xpkClose(xpk);
}

TEST(error_callback_registration) {
	xpkObject xpk = xpkOpen("test_11_callback.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);
}

void register_11_error_handling_tests(void) {
	TEST_REGISTER(error_null_pointer, CAT_ERROR, "Test null pointer error");
	TEST_REGISTER(error_invalid_path, CAT_ERROR, "Test invalid path error");
	TEST_REGISTER(error_readonly_write, CAT_ERROR, "Test readonly write error");
	TEST_REGISTER(error_out_of_range, CAT_ERROR, "Test out of range error");
	TEST_REGISTER(error_invalid_level, CAT_ERROR, "Test invalid compression level");
	TEST_REGISTER(error_empty_data, CAT_ERROR, "Test empty data handling");
	TEST_REGISTER(error_remove_nonexistent, CAT_ERROR, "Test remove nonexistent error");
	TEST_REGISTER(error_update_nonexistent, CAT_ERROR, "Test update nonexistent error");
	TEST_REGISTER(error_corrupted_package, CAT_ERROR, "Test corrupted package error");
	TEST_REGISTER(error_invalid_signature, CAT_ERROR, "Test invalid signature error");
	TEST_REGISTER(error_unsupported_version, CAT_ERROR, "Test unsupported version error");
	TEST_REGISTER(error_file_not_found, CAT_ERROR, "Test file not found error");
	TEST_REGISTER(error_invalid_position, CAT_ERROR, "Test invalid position error");
	TEST_REGISTER(error_null_object, CAT_ERROR, "Test null object error");
	TEST_REGISTER(error_pack_type_change_after_files, CAT_ERROR, "Test pack type change after files");
	TEST_REGISTER(error_index_not_found, CAT_ERROR, "Test index not found error");
	TEST_REGISTER(error_index_duplicate, CAT_ERROR, "Test index duplicate error");
	TEST_REGISTER(error_path_duplicate, CAT_ERROR, "Test path duplicate error");
	TEST_REGISTER(error_path_too_long, CAT_ERROR, "Test path too long error");
	TEST_REGISTER(error_null_parameters, CAT_ERROR, "Test null parameters error");
	TEST_REGISTER(error_wrong_pack_type, CAT_ERROR, "Test wrong pack type error");
	TEST_REGISTER(error_message_clearing, CAT_ERROR, "Test error message clearing");
	TEST_REGISTER(error_callback_registration, CAT_ERROR, "Test callback registration");
}
