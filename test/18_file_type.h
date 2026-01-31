/*
 * xPack Ver7 - file_type (18)
 */

#include "test_framework.h"

TEST(ftype_default) {
	xpkObject xpk = xpkOpen("test_18_ftype_default.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Data", 4, 6);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_UNKNOWN);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_18_ftype_default.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_UNKNOWN);
	xpkClose(xpk);
}

TEST(ftype_set_binary) {
	xpkObject xpk = xpkOpen("test_18_ftype_binary.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Binary data", 11, 6);
	ASSERT_EQ(xpkInfoTypeSet(xpk, 0, XPK_FTYPE_BINARY), 0);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_BINARY);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_18_ftype_binary.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_BINARY);
	xpkClose(xpk);
}

TEST(ftype_set_text) {
	xpkObject xpk = xpkOpen("test_18_ftype_text.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Text data", 9, 6);
	ASSERT_EQ(xpkInfoTypeSet(xpk, 0, XPK_FTYPE_TEXT), 0);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_TEXT);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_18_ftype_text.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_TEXT);
	xpkClose(xpk);
}

TEST(ftype_set_image) {
	xpkObject xpk = xpkOpen("test_18_ftype_image.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Image data", 10, 6);
	ASSERT_EQ(xpkInfoTypeSet(xpk, 0, XPK_FTYPE_IMAGE), 0);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_IMAGE);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_18_ftype_image.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_IMAGE);
	xpkClose(xpk);
}

TEST(ftype_set_audio) {
	xpkObject xpk = xpkOpen("test_18_ftype_audio.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Audio data", 10, 6);
	ASSERT_EQ(xpkInfoTypeSet(xpk, 0, XPK_FTYPE_AUDIO), 0);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_AUDIO);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_18_ftype_audio.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_AUDIO);
	xpkClose(xpk);
}

TEST(ftype_set_video) {
	xpkObject xpk = xpkOpen("test_18_ftype_video.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Video data", 10, 6);
	ASSERT_EQ(xpkInfoTypeSet(xpk, 0, XPK_FTYPE_VIDEO), 0);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_VIDEO);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_18_ftype_video.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_VIDEO);
	xpkClose(xpk);
}

TEST(ftype_set_archive) {
	xpkObject xpk = xpkOpen("test_18_ftype_archive.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Archive data", 12, 6);
	ASSERT_EQ(xpkInfoTypeSet(xpk, 0, XPK_FTYPE_ARCHIVE), 0);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_ARCHIVE);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_18_ftype_archive.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_ARCHIVE);
	xpkClose(xpk);
}

TEST(ftype_set_executable) {
	xpkObject xpk = xpkOpen("test_18_ftype_exec.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Executable data", 15, 6);
	ASSERT_EQ(xpkInfoTypeSet(xpk, 0, XPK_FTYPE_ARCHIVE), 0);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_ARCHIVE);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_18_ftype_exec.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_ARCHIVE);
	xpkClose(xpk);
}

TEST(ftype_multiple_types) {
	xpkObject xpk = xpkOpen("test_18_ftype_multiple.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	int types[] = {
		XPK_FTYPE_UNKNOWN,
		XPK_FTYPE_BINARY,
		XPK_FTYPE_TEXT,
		XPK_FTYPE_IMAGE,
		XPK_FTYPE_AUDIO,
		XPK_FTYPE_VIDEO,
		XPK_FTYPE_ARCHIVE,
		XPK_FTYPE_ARCHIVE
	};

	for (int i = 0; i < 8; i++) {
		xpkAppendData(xpk, "Data", 4, 6);
		ASSERT_EQ(xpkInfoTypeSet(xpk, i, types[i]), 0);
		ASSERT_EQ(xpkInfoType(xpk, i), types[i]);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_18_ftype_multiple.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 8; i++) {
		ASSERT_EQ(xpkInfoType(xpk, i), types[i]);
	}

	xpkClose(xpk);
}

TEST(ftype_change_type) {
	xpkObject xpk = xpkOpen("test_18_ftype_change.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Data", 4, 6);

	ASSERT_EQ(xpkInfoTypeSet(xpk, 0, XPK_FTYPE_TEXT), 0);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_TEXT);

	ASSERT_EQ(xpkInfoTypeSet(xpk, 0, XPK_FTYPE_BINARY), 0);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_BINARY);

	ASSERT_EQ(xpkInfoTypeSet(xpk, 0, XPK_FTYPE_ARCHIVE), 0);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_ARCHIVE);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_18_ftype_change.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_ARCHIVE);
	xpkClose(xpk);
}

TEST(ftype_after_update) {
	xpkObject xpk = xpkOpen("test_18_ftype_update.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Original", 8, 6);
	ASSERT_EQ(xpkInfoTypeSet(xpk, 0, XPK_FTYPE_TEXT), 0);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_18_ftype_update.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_TEXT);

	ASSERT_EQ(xpkUpdateData(xpk, 0, "Updated", 7, 6), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_18_ftype_update.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_TEXT);
	xpkClose(xpk);
}

TEST(ftype_all_constants) {
	int allTypes[] = {
		XPK_FTYPE_UNKNOWN,
		XPK_FTYPE_BINARY,
		XPK_FTYPE_TEXT,
		XPK_FTYPE_IMAGE,
		XPK_FTYPE_AUDIO,
		XPK_FTYPE_VIDEO,
		XPK_FTYPE_ARCHIVE,
		XPK_FTYPE_ARCHIVE
	};

	for (int i = 0; i < 8; i++) {
		char filename[64];
		sprintf(filename, "test_18_ftype_const_%d.xpk", i);

		xpkObject xpk = xpkOpen(filename, 0, 0);
		ASSERT_NOT_NULL(xpk);

		xpkAppendData(xpk, "Data", 4, 6);
		ASSERT_EQ(xpkInfoTypeSet(xpk, 0, allTypes[i]), 0);
		ASSERT_EQ(xpkInfoType(xpk, 0), allTypes[i]);

		ASSERT_EQ(xpkSave(xpk), 0);
		xpkClose(xpk);

		xpk = xpkOpen(filename, 0, 1);
		ASSERT_NOT_NULL(xpk);
		ASSERT_EQ(xpkInfoType(xpk, 0), allTypes[i]);
		xpkClose(xpk);
	}
}

TEST(ftype_invalid_position) {
	xpkObject xpk = xpkOpen("test_18_ftype_invalid.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Data", 4, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_18_ftype_invalid.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkInfoType(xpk, 1), -1);
	ASSERT_EQ(xpkInfoTypeSet(xpk, 1, XPK_FTYPE_TEXT), -1);

	xpkClose(xpk);
}

TEST(ftype_null_object) {
	ASSERT_EQ(xpkInfoType(NULL, 0), -1);
	ASSERT_EQ(xpkInfoTypeSet(NULL, 0, XPK_FTYPE_TEXT), -1);
}

TEST(ftype_path_mode) {
	xpkObject xpk = xpkOpen("test_18_ftype_path.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	xpkPathAppendData(xpk, "file.txt", "Data", 4, 6);
	ASSERT_EQ(xpkInfoTypeSet(xpk, 0, XPK_FTYPE_TEXT), 0);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_TEXT);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_18_ftype_path.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_TEXT);
	xpkClose(xpk);
}

TEST(ftype_index_mode) {
	xpkObject xpk = xpkOpen("test_18_ftype_index.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

	xpkIndexAppendData(xpk, 100, "Data", 4, 6);
	ASSERT_EQ(xpkInfoTypeSet(xpk, 0, XPK_FTYPE_ARCHIVE), 0);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_ARCHIVE);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_18_ftype_index.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_ARCHIVE);
	xpkClose(xpk);
}

TEST(ftype_preserves_after_rebuild) {
	xpkObject xpk = xpkOpen("test_18_ftype_rebuild.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 5; i++) {
		xpkAppendData(xpk, "Data", 4, 6);
		ASSERT_EQ(xpkInfoTypeSet(xpk, i, i + 1), 0);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_18_ftype_rebuild.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkRebuild(xpk), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_18_ftype_rebuild.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 5; i++) {
		ASSERT_EQ(xpkInfoType(xpk, i), i + 1);
	}

	xpkClose(xpk);
}

TEST(ftype_readonly_preserves) {
	xpkObject xpk = xpkOpen("test_18_ftype_readonly.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Data", 4, 6);
	ASSERT_EQ(xpkInfoTypeSet(xpk, 0, XPK_FTYPE_IMAGE), 0);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_18_ftype_readonly.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_IMAGE);

	ASSERT_EQ(xpkInfoTypeSet(xpk, 0, XPK_FTYPE_TEXT), -1);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_IMAGE);

	xpkClose(xpk);
}

void register_18_file_type_tests(void) {
	TEST_REGISTER(ftype_default, CAT_CORE, "Test default file type");
	TEST_REGISTER(ftype_set_binary, CAT_CORE, "Test set binary file type");
	TEST_REGISTER(ftype_set_text, CAT_CORE, "Test set text file type");
	TEST_REGISTER(ftype_set_image, CAT_CORE, "Test set image file type");
	TEST_REGISTER(ftype_set_audio, CAT_CORE, "Test set audio file type");
	TEST_REGISTER(ftype_set_video, CAT_CORE, "Test set video file type");
	TEST_REGISTER(ftype_set_archive, CAT_CORE, "Test set archive file type");
	TEST_REGISTER(ftype_set_executable, CAT_CORE, "Test set executable file type");
	TEST_REGISTER(ftype_multiple_types, CAT_CORE, "Test multiple file types");
	TEST_REGISTER(ftype_change_type, CAT_CORE, "Test change file type");
	TEST_REGISTER(ftype_after_update, CAT_CORE, "Test file type after update");
	TEST_REGISTER(ftype_all_constants, CAT_CORE, "Test all file type constants");
	TEST_REGISTER(ftype_invalid_position, CAT_CORE, "Test file type with invalid position");
	TEST_REGISTER(ftype_null_object, CAT_CORE, "Test file type with null object");
	TEST_REGISTER(ftype_path_mode, CAT_CORE, "Test file type in path mode");
	TEST_REGISTER(ftype_index_mode, CAT_CORE, "Test file type in index mode");
	TEST_REGISTER(ftype_preserves_after_rebuild, CAT_CORE, "Test file type preserves after rebuild");
	TEST_REGISTER(ftype_readonly_preserves, CAT_CORE, "Test file type preserves in readonly mode");
}
