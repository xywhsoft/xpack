/*
 * xPack Ver7 - 包属性测试
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

TEST(prop_type_default) {
	xpkObject xpk = xpkOpen("test_17_type_default.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkType(xpk), XPK_TYPE_CORE);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_type_default.xpk", 0, 1);
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
	xpkObject xpk = xpkOpen("test_17_type_change.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);

	xpkAppendData(xpk, "Data", 4, 6);

	ASSERT_NE(xpkTypeSet(xpk, XPK_TYPE_LINUX), 0);
	ASSERT_EQ(xpkType(xpk), XPK_TYPE_INDEX);

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

	for (int i = 0; i < 10; i++) {
		ASSERT_EQ(xpkCount(xpk), i);
		xpkAppendData(xpk, "Data", 4, 6);
		ASSERT_EQ(xpkCount(xpk), i + 1);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_count_add.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 10);
	xpkClose(xpk);
}

TEST(prop_count_remove_files) {
	xpkObject xpk = xpkOpen("test_17_count_remove.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 10; i++) {
		xpkAppendData(xpk, "Data", 4, 6);
	}

	ASSERT_EQ(xpkCount(xpk), 10);

	ASSERT_EQ(xpkRemove(xpk, 2), 0);
	ASSERT_EQ(xpkCount(xpk), 9);

	ASSERT_EQ(xpkRemove(xpk, 5), 0);
	ASSERT_EQ(xpkCount(xpk), 8);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_count_remove.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkCount(xpk), 8);
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

	ASSERT_EQ(xpkDiscCodeSet(xpk, 0xABCDEF01), 0);
	ASSERT_EQ(xpkDiscCode(xpk), 0xABCDEF01);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_disc_set.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);
	ASSERT_EQ(xpkDiscCode(xpk), 0xABCDEF01);
	xpkClose(xpk);
}

TEST(prop_disc_code_zero) {
	xpkObject xpk = xpkOpen("test_17_disc_zero.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkDiscCodeSet(xpk, 0xFFFFFFFF), 0);
	ASSERT_EQ(xpkDiscCode(xpk), 0xFFFFFFFF);

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

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);
	ASSERT_EQ(xpkDiscCodeSet(xpk, 0xDEADBEEF), 0);

	xpkAppendData(xpk, "Data", 4, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_get_head.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	xpkHead* head = xpkGetHead(xpk);
	ASSERT_NOT_NULL(head);

	ASSERT_EQ(head->signature, XPK_SIGNATURE);
	ASSERT_EQ(head->version, 7);
	ASSERT_EQ(head->packType, XPK_TYPE_INDEX);
	ASSERT_EQ(head->discCode, 0xDEADBEEF);
	ASSERT_EQ(head->fileCount, 1);

	xpkClose(xpk);
}

TEST(prop_all_types) {
	for (int type = 0; type <= 3; type++) {
		char filename[64];
		sprintf(filename, "test_17_type_%d.xpk", type);

		xpkObject xpk = xpkOpen(filename, 0, 0);
		ASSERT_NOT_NULL(xpk);

		ASSERT_EQ(xpkTypeSet(xpk, type), 0);
		ASSERT_EQ(xpkType(xpk), type);

		ASSERT_EQ(xpkSave(xpk), 0);
		xpkClose(xpk);

		xpk = xpkOpen(filename, 0, 1);
		ASSERT_NOT_NULL(xpk);
		ASSERT_EQ(xpkType(xpk), type);
		xpkClose(xpk);
	}
}

TEST(prop_version) {
	xpkObject xpk = xpkOpen("test_17_version.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Data", 4, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_version.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	xpkHead* head = xpkGetHead(xpk);
	ASSERT_NOT_NULL(head);
	ASSERT_EQ(head->version, 7);

	xpkClose(xpk);
}

TEST(prop_signature) {
	xpkObject xpk = xpkOpen("test_17_signature.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Data", 4, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_signature.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	xpkHead* head = xpkGetHead(xpk);
	ASSERT_NOT_NULL(head);
	ASSERT_EQ(head->signature, XPK_SIGNATURE);

	xpkClose(xpk);
}

TEST(prop_multiple_operations) {
	xpkObject xpk = xpkOpen("test_17_multi_ops.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);
	ASSERT_EQ(xpkDiscCodeSet(xpk, 0x12345678), 0);
	ASSERT_EQ(xpkType(xpk), XPK_TYPE_WIN32);
	ASSERT_EQ(xpkDiscCode(xpk), 0x12345678);

	for (int i = 0; i < 5; i++) {
		ASSERT_EQ(xpkCount(xpk), i);
		xpkAppendData(xpk, "Data", 4, 6);
		ASSERT_EQ(xpkCount(xpk), i + 1);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_multi_ops.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkType(xpk), XPK_TYPE_WIN32);
	ASSERT_EQ(xpkDiscCode(xpk), 0x12345678);
	ASSERT_EQ(xpkCount(xpk), 5);

	xpkClose(xpk);
}

TEST(prop_readonly_preserves) {
	xpkObject xpk = xpkOpen("test_17_readonly_preserves.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_LINUX), 0);
	ASSERT_EQ(xpkDiscCodeSet(xpk, 0xFEDCBA98), 0);

	xpkAppendData(xpk, "Data", 4, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_17_readonly_preserves.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkType(xpk), XPK_TYPE_LINUX);
	ASSERT_EQ(xpkDiscCode(xpk), 0xFEDCBA98);
	ASSERT_EQ(xpkCount(xpk), 1);

	xpkClose(xpk);
}

int main(void) {
	xrtInit();

	printf("=================================================\n");
	printf("  xPack Ver7 - Package Properties Test\n");
	printf("=================================================\n\n");

	printf("[Package Type Tests]\n");
	RUN_TEST(prop_type_default);
	RUN_TEST(prop_type_set_index);
	RUN_TEST(prop_type_set_linux);
	RUN_TEST(prop_type_set_win32);
	RUN_TEST(prop_type_change_with_files);
	RUN_TEST(prop_all_types);
	printf("\n");

	printf("[Package Count Tests]\n");
	RUN_TEST(prop_count_empty);
	RUN_TEST(prop_count_add_files);
	RUN_TEST(prop_count_remove_files);
	printf("\n");

	printf("[Package Disc Code Tests]\n");
	RUN_TEST(prop_disc_code_default);
	RUN_TEST(prop_disc_code_set);
	RUN_TEST(prop_disc_code_zero);
	printf("\n");

	printf("[Package Header Tests]\n");
	RUN_TEST(prop_get_head);
	RUN_TEST(prop_version);
	RUN_TEST(prop_signature);
	printf("\n");

	printf("[Property Preservation Tests]\n");
	RUN_TEST(prop_multiple_operations);
	RUN_TEST(prop_readonly_preserves);
	printf("\n");

	printf("=================================================\n");
	printf("  Results: %d passed, %d failed\n", tests_passed, tests_failed);
	printf("=================================================\n");

	return tests_failed > 0 ? 1 : 0;
}
