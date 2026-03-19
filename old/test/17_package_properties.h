/*
 * xPack Ver7 - package_properties (17)
 */

#include "test_framework.h"

TEST(props_type) {
	xpkObject xpk = xpkOpen("test_17_type.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkType(xpk), XPK_TYPE_CORE);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_type.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkType(xpk), XPK_TYPE_WIN32);
	xpkClose(xpk);
}

TEST(props_count) {
	xpkObject xpk = xpkOpen("test_17_count.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkCount(xpk), 0);

	for (int i = 0; i < 10; i++) {
		char data[64];
		sprintf(data, "File %d", i);
		ASSERT_NE(xpkAppendData(xpk, data, (uint32_t)strlen(data), 6), UINT32_MAX);
		ASSERT_EQ(xpkCount(xpk), i + 1);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 10);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_count.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 10);
	xpkClose(xpk);
}

TEST(props_solid_mode) {
	xpkObject xpk = xpkOpen("test_17_solid.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkSolidMode(xpk), 0);

	for (int i = 0; i < 5; i++) {
		char data[256];
		memset(data, 'A' + i, 256);
		ASSERT_NE(xpkAppendData(xpk, data, 256, 6), UINT32_MAX);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_solid.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 5);
	xpkClose(xpk);
}

TEST(props_volume_mode) {
	xpkObject xpk = xpkOpen("test_17_volume.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_volume.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkVolumeMode(xpk), 0);
	xpkClose(xpk);
}

TEST(props_info_access) {
	xpkObject xpk = xpkOpen("test_17_info.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	char testData[1024];
	memset(testData, 'X', sizeof(testData));

	ASSERT_NE(xpkAppendData(xpk, testData, sizeof(testData), 9), UINT32_MAX);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_info.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkCount(xpk), 1);
	ASSERT_EQ(xpkInfoSize(xpk, 0), 1024);
	ASSERT_GT(xpkInfoPacked(xpk, 0), 0);
	ASSERT_EQ(xpkInfoLevel(xpk, 0), 9);
	ASSERT_EQ(xpkInfoType(xpk, 0), XPK_FTYPE_UNKNOWN);
	ASSERT_NE(xpkInfoHash(xpk, 0), 0);

	xpkClose(xpk);
}

TEST(props_multiple_properties) {
	xpkObject xpk = xpkOpen("test_17_multi_props.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	for (int i = 0; i < 3; i++) {
		char data[512];
		sprintf(data, "Property file %d", i);
		ASSERT_NE(xpkAppendData(xpk, data, (uint32_t)strlen(data), 6), UINT32_MAX);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	ASSERT_EQ(xpkCount(xpk), 3);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_multi_props.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkType(xpk), XPK_TYPE_WIN32);
	ASSERT_EQ(xpkCount(xpk), 3);

	for (uint32_t i = 0; i < 3; i++) {
		ASSERT_GT(xpkInfoSize(xpk, i), 0);
		ASSERT_GT(xpkInfoPacked(xpk, i), 0);
		ASSERT_EQ(xpkInfoLevel(xpk, i), 6);
		ASSERT_NE(xpkInfoHash(xpk, i), 0);
	}

	xpkClose(xpk);
}

TEST(props_type_persistence) {
	xpkObject xpk = xpkOpen("test_17_persist.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_LINUX), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_persist.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkType(xpk), XPK_TYPE_LINUX);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_persist.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_persist.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkType(xpk), XPK_TYPE_WIN32);
	xpkClose(xpk);
}

TEST(props_empty_package) {
	xpkObject xpk = xpkOpen("test_17_empty.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkCount(xpk), 0);
	ASSERT_EQ(xpkType(xpk), XPK_TYPE_CORE);
	ASSERT_EQ(xpkSolidMode(xpk), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_empty.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 0);
	ASSERT_EQ(xpkType(xpk), XPK_TYPE_CORE);
	xpkClose(xpk);
}

TEST(prop_type_default) {
	xpkObject xpk = xpkOpen("test_17_type_default.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkType(xpk), XPK_TYPE_CORE);
	xpkClose(xpk);
}

TEST(prop_type_set_index) {
	xpkObject xpk = xpkOpen("test_17_type_index.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);
	ASSERT_EQ(xpkType(xpk), XPK_TYPE_INDEX);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_type_index.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkType(xpk), XPK_TYPE_INDEX);
	xpkClose(xpk);
}

TEST(prop_type_set_linux) {
	xpkObject xpk = xpkOpen("test_17_type_linux.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_LINUX), 0);
	ASSERT_EQ(xpkType(xpk), XPK_TYPE_LINUX);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_type_linux.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkType(xpk), XPK_TYPE_LINUX);
	xpkClose(xpk);
}

TEST(prop_type_set_win32) {
	xpkObject xpk = xpkOpen("test_17_type_win32.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);
	ASSERT_EQ(xpkType(xpk), XPK_TYPE_WIN32);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_type_win32.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkType(xpk), XPK_TYPE_WIN32);
	xpkClose(xpk);
}

TEST(prop_type_change_with_files) {
	xpkObject xpk = xpkOpen("test_17_type_change_files.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_CORE), 0);
	xpkAppendData(xpk, "File1", 5, 6);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), -1);
	ASSERT_EQ(xpkType(xpk), XPK_TYPE_CORE);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);
}

TEST(prop_count_empty) {
	xpkObject xpk = xpkOpen("test_17_count_empty.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkCount(xpk), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_count_empty.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 0);
	xpkClose(xpk);
}

TEST(prop_count_add_files) {
	xpkObject xpk = xpkOpen("test_17_count_add.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkCount(xpk), 0);

	for (int i = 0; i < 100; i++) {
		char data[32];
		sprintf(data, "File%d", i);
		ASSERT_NE(xpkAppendData(xpk, data, (uint32_t)strlen(data), 6), UINT32_MAX);
		ASSERT_EQ(xpkCount(xpk), i + 1);
	}

	ASSERT_EQ(xpkCount(xpk), 100);
	xpkClose(xpk);
}

TEST(prop_count_remove_files) {
	xpkObject xpk = xpkOpen("test_17_count_remove.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 10; i++) {
		char data[32];
		sprintf(data, "File%d", i);
		ASSERT_NE(xpkAppendData(xpk, data, (uint32_t)strlen(data), 6), UINT32_MAX);
	}

	ASSERT_EQ(xpkCount(xpk), 10);

	for (int i = 9; i >= 0; i--) {
		ASSERT_EQ(xpkRemove(xpk, i), 0);
		ASSERT_EQ(xpkCount(xpk), i);
	}

	ASSERT_EQ(xpkCount(xpk), 0);
	xpkClose(xpk);
}

TEST(prop_disc_code_default) {
	xpkObject xpk = xpkOpen("test_17_disc_default.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkDiscCode(xpk), 0);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_disc_default.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkDiscCode(xpk), 0);
	xpkClose(xpk);
}

TEST(prop_disc_code_set) {
	xpkObject xpk = xpkOpen("test_17_disc_set.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkDiscCodeSet(xpk, 0x12345678), 0);
	ASSERT_EQ(xpkDiscCode(xpk), 0x12345678);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_disc_set.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkDiscCode(xpk), 0x12345678);
	xpkClose(xpk);
}

TEST(prop_disc_code_zero) {
	xpkObject xpk = xpkOpen("test_17_disc_zero.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkDiscCodeSet(xpk, 0xABCDEF01), 0);
	ASSERT_EQ(xpkDiscCode(xpk), 0xABCDEF01);

	ASSERT_EQ(xpkDiscCodeSet(xpk, 0), 0);
	ASSERT_EQ(xpkDiscCode(xpk), 0);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_disc_zero.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkDiscCode(xpk), 0);
	xpkClose(xpk);
}

TEST(prop_get_head) {
	xpkObject xpk = xpkOpen("test_17_get_head.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	char testData[256];
	memset(testData, 'H', sizeof(testData));
	ASSERT_NE(xpkAppendData(xpk, testData, sizeof(testData), 6), UINT32_MAX);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_get_head.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	xpkHead* head = xpkGetHead(xpk);
	ASSERT_NOT_NULL(head);
	ASSERT_EQ(head->fileHead, XPK_VERSION);

	xpkClose(xpk);
}

TEST(prop_all_types) {
	int types[] = {XPK_TYPE_CORE, XPK_TYPE_WIN32, XPK_TYPE_LINUX, XPK_TYPE_INDEX};

	for (int i = 0; i < 4; i++) {
		char filename[64];
		sprintf(filename, "test_17_all_types_%d.xpk", i);

		xpkObject xpk = xpkOpen(filename, 0, 0);
		ASSERT_NOT_NULL(xpk);

		ASSERT_EQ(xpkTypeSet(xpk, types[i]), 0);
		ASSERT_EQ(xpkType(xpk), types[i]);

		ASSERT_EQ(xpkSave(xpk), 0);
		xpkClose(xpk);

		xpk = xpkOpen(filename, 0, 1);
		ASSERT_NOT_NULL(xpk);
		ASSERT_EQ(xpkType(xpk), types[i]);
		xpkClose(xpk);
	}
}

TEST(prop_version) {
	xpkObject xpk = xpkOpen("test_17_version.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_version.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	xpkHead* head = xpkGetHead(xpk);
	ASSERT_NOT_NULL(head);
	ASSERT_EQ(head->fileHead, XPK_VERSION);

	xpkClose(xpk);
}

TEST(prop_signature) {
	xpkObject xpk = xpkOpen("test_17_signature.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_signature.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	xpkHead* head = xpkGetHead(xpk);
	ASSERT_NOT_NULL(head);
	ASSERT_EQ(head->fileHead, XPK_VERSION);

	xpkClose(xpk);
}

TEST(prop_multiple_operations) {
	xpkObject xpk = xpkOpen("test_17_multi_ops.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);
	ASSERT_EQ(xpkDiscCodeSet(xpk, 0xDEADBEEF), 0);

	for (int i = 0; i < 5; i++) {
		char data[128];
		sprintf(data, "Operation%d", i);
		ASSERT_NE(xpkAppendData(xpk, data, (uint32_t)strlen(data), 6), UINT32_MAX);
	}

	ASSERT_EQ(xpkCount(xpk), 5);
	ASSERT_EQ(xpkType(xpk), XPK_TYPE_WIN32);
	ASSERT_EQ(xpkDiscCode(xpk), 0xDEADBEEF);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_multi_ops.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkCount(xpk), 5);
	ASSERT_EQ(xpkType(xpk), XPK_TYPE_WIN32);
	ASSERT_EQ(xpkDiscCode(xpk), 0xDEADBEEF);

	xpkHead* head = xpkGetHead(xpk);
	ASSERT_NOT_NULL(head);
	ASSERT_EQ(head->fileHead, XPK_VERSION);

	xpkClose(xpk);
}

TEST(prop_readonly_preserves) {
	xpkObject xpk = xpkOpen("test_17_readonly_preserve.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_LINUX), 0);
	ASSERT_EQ(xpkDiscCodeSet(xpk, 0x11223344), 0);
	ASSERT_NE(xpkAppendData(xpk, "Data", 4, 6), UINT32_MAX);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_readonly_preserve.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkType(xpk), XPK_TYPE_LINUX);
	ASSERT_EQ(xpkDiscCode(xpk), 0x11223344);
	ASSERT_EQ(xpkCount(xpk), 1);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), -1);
	ASSERT_EQ(xpkDiscCodeSet(xpk, 0x55667788), -1);

	ASSERT_EQ(xpkType(xpk), XPK_TYPE_LINUX);
	ASSERT_EQ(xpkDiscCode(xpk), 0x11223344);

	xpkClose(xpk);
}

void register_17_package_properties_tests(void) {
	TEST_REGISTER(props_type, CAT_CORE, "Test package type property");
	TEST_REGISTER(props_count, CAT_CORE, "Test package count property");
	TEST_REGISTER(props_solid_mode, CAT_CORE, "Test package solid mode property");
	TEST_REGISTER(props_volume_mode, CAT_CORE, "Test package volume mode property");
	TEST_REGISTER(props_info_access, CAT_CORE, "Test package info access");
	TEST_REGISTER(props_multiple_properties, CAT_CORE, "Test multiple properties");
	TEST_REGISTER(props_type_persistence, CAT_CORE, "Test property persistence");
	TEST_REGISTER(props_empty_package, CAT_CORE, "Test empty package properties");
	TEST_REGISTER(prop_type_default, CAT_CORE, "Test default package type");
	TEST_REGISTER(prop_type_set_index, CAT_CORE, "Test set index type");
	TEST_REGISTER(prop_type_set_linux, CAT_CORE, "Test set Linux type");
	TEST_REGISTER(prop_type_set_win32, CAT_CORE, "Test set Win32 type");
	TEST_REGISTER(prop_type_change_with_files, CAT_CORE, "Test type change with files");
	TEST_REGISTER(prop_count_empty, CAT_CORE, "Test count empty package");
	TEST_REGISTER(prop_count_add_files, CAT_CORE, "Test count after add files");
	TEST_REGISTER(prop_count_remove_files, CAT_CORE, "Test count after remove files");
	TEST_REGISTER(prop_disc_code_default, CAT_CORE, "Test default disc code");
	TEST_REGISTER(prop_disc_code_set, CAT_CORE, "Test set disc code");
	TEST_REGISTER(prop_disc_code_zero, CAT_CORE, "Test zero disc code");
	TEST_REGISTER(prop_get_head, CAT_CORE, "Test get head");
	TEST_REGISTER(prop_all_types, CAT_CORE, "Test all types");
	TEST_REGISTER(prop_version, CAT_CORE, "Test version");
	TEST_REGISTER(prop_signature, CAT_CORE, "Test signature");
	TEST_REGISTER(prop_multiple_operations, CAT_CORE, "Test multiple operations");
	TEST_REGISTER(prop_readonly_preserves, CAT_CORE, "Test readonly preserves");
}
