/*
 * xPack Ver7 - 遍历操作测试
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

typedef struct {
	int callCount;
	uint32_t lastPos;
} TraverseContext;

static int eachCallback(void* ctx, uint32_t pos, void* info) {
	TraverseContext* context = (TraverseContext*)ctx;
	context->callCount++;
	context->lastPos = pos;
	return 1;
}

static int eachMatchCallback(void* ctx, uint32_t pos, void* info) {
	TraverseContext* context = (TraverseContext*)ctx;
	context->callCount++;
	context->lastPos = pos;
	return 1;
}

static int stopEarlyCallback(void* ctx, uint32_t pos, void* info) {
	TraverseContext* context = (TraverseContext*)ctx;
	context->callCount++;
	if (context->callCount >= 2) {
		return 0;
	}
	return 1;
}

TEST(traverse_basic) {
	xpkObject xpk = xpkOpen("test_13_traverse_basic.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Data 1", 6, 6);
	xpkAppendData(xpk, "Data 2", 6, 6);
	xpkAppendData(xpk, "Data 3", 6, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_13_traverse_basic.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	TraverseContext ctx = { 0, 0 };
	int result = xpkEach(xpk, &ctx, eachCallback);
	ASSERT_EQ(result, 3);
	ASSERT_EQ(ctx.callCount, 3);

	xpkClose(xpk);
}

TEST(traverse_empty) {
	xpkObject xpk = xpkOpen("test_13_traverse_empty.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkCount(xpk), 0);
	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_13_traverse_empty.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	TraverseContext ctx = { 0, 0 };
	int result = xpkEach(xpk, &ctx, eachCallback);
	ASSERT_EQ(result, 0);
	ASSERT_EQ(ctx.callCount, 0);

	xpkClose(xpk);
}

TEST(traverse_single) {
	xpkObject xpk = xpkOpen("test_13_traverse_single.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	xpkAppendData(xpk, "Single file", 11, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_13_traverse_single.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	TraverseContext ctx = { 0, 0 };
	int result = xpkEach(xpk, &ctx, eachCallback);
	ASSERT_EQ(result, 1);
	ASSERT_EQ(ctx.callCount, 1);
	ASSERT_EQ(ctx.lastPos, 0);

	xpkClose(xpk);
}

TEST(traverse_stop_early) {
	xpkObject xpk = xpkOpen("test_13_traverse_stop.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	for (int i = 0; i < 10; i++) {
		xpkAppendData(xpk, "Data", 4, 6);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_13_traverse_stop.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	TraverseContext ctx = { 0, 0 };
	int result = xpkEach(xpk, &ctx, stopEarlyCallback);
	ASSERT_EQ(result, 2);
	ASSERT_EQ(ctx.callCount, 2);

	xpkClose(xpk);
}

TEST(traverse_match_star) {
	xpkObject xpk = xpkOpen("test_13_match_star.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	xpkPathAppendData(xpk, "file1.txt", "Data 1", 6, 6);
	xpkPathAppendData(xpk, "file2.txt", "Data 2", 6, 6);
	xpkPathAppendData(xpk, "data.bin", "Data 3", 6, 6);
	xpkPathAppendData(xpk, "file3.txt", "Data 4", 6, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_13_match_star.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	TraverseContext ctx = { 0, 0 };
	int result = xpkEachMatch(xpk, "*.txt", &ctx, eachMatchCallback);
	ASSERT_EQ(result, 3);
	ASSERT_EQ(ctx.callCount, 3);

	xpkClose(xpk);
}

TEST(traverse_match_question) {
	xpkObject xpk = xpkOpen("test_13_match_question.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	xpkPathAppendData(xpk, "file1.txt", "Data 1", 6, 6);
	xpkPathAppendData(xpk, "file2.txt", "Data 2", 6, 6);
	xpkPathAppendData(xpk, "file12.txt", "Data 3", 6, 6);
	xpkPathAppendData(xpk, "file.txt", "Data 4", 6, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_13_match_question.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	TraverseContext ctx = { 0, 0 };
	int result = xpkEachMatch(xpk, "file?.txt", &ctx, eachMatchCallback);
	ASSERT_EQ(result, 2);
	ASSERT_EQ(ctx.callCount, 2);

	xpkClose(xpk);
}

TEST(traverse_match_extension) {
	xpkObject xpk = xpkOpen("test_13_match_ext.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	xpkPathAppendData(xpk, "doc1.txt", "Text 1", 6, 6);
	xpkPathAppendData(xpk, "doc2.txt", "Text 2", 6, 6);
	xpkPathAppendData(xpk, "image.png", "PNG", 3, 6);
	xpkPathAppendData(xpk, "data.bin", "Binary", 6, 6);
	xpkPathAppendData(xpk, "log.txt", "Log", 3, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_13_match_ext.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	TraverseContext ctx = { 0, 0 };
	int result = xpkEachMatch(xpk, "*.txt", &ctx, eachMatchCallback);
	ASSERT_EQ(result, 3);
	ASSERT_EQ(ctx.callCount, 3);

	xpkClose(xpk);
}

TEST(traverse_match_prefix) {
	xpkObject xpk = xpkOpen("test_13_match_prefix.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	xpkPathAppendData(xpk, "test1.txt", "Data 1", 6, 6);
	xpkPathAppendData(xpk, "test2.txt", "Data 2", 6, 6);
	xpkPathAppendData(xpk, "other.txt", "Data 3", 6, 6);
	xpkPathAppendData(xpk, "test3.txt", "Data 4", 6, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_13_match_prefix.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	TraverseContext ctx = { 0, 0 };
	int result = xpkEachMatch(xpk, "test*.txt", &ctx, eachMatchCallback);
	ASSERT_EQ(result, 3);
	ASSERT_EQ(ctx.callCount, 3);

	xpkClose(xpk);
}

TEST(traverse_match_complex) {
	xpkObject xpk = xpkOpen("test_13_match_complex.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	xpkPathAppendData(xpk, "file001.txt", "Data 1", 6, 6);
	xpkPathAppendData(xpk, "file002.txt", "Data 2", 6, 6);
	xpkPathAppendData(xpk, "file123.txt", "Data 3", 6, 6);
	xpkPathAppendData(xpk, "file999.txt", "Data 4", 6, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_13_match_complex.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	TraverseContext ctx = { 0, 0 };
	int result = xpkEachMatch(xpk, "file???.txt", &ctx, eachMatchCallback);
	ASSERT_EQ(result, 4);
	ASSERT_EQ(ctx.callCount, 4);

	xpkClose(xpk);
}

TEST(traverse_match_no_matches) {
	xpkObject xpk = xpkOpen("test_13_match_none.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	xpkPathAppendData(xpk, "file1.txt", "Data 1", 6, 6);
	xpkPathAppendData(xpk, "file2.txt", "Data 2", 6, 6);
	xpkPathAppendData(xpk, "data.bin", "Data 3", 6, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_13_match_none.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	TraverseContext ctx = { 0, 0 };
	int result = xpkEachMatch(xpk, "*.log", &ctx, eachMatchCallback);
	ASSERT_EQ(result, 0);
	ASSERT_EQ(ctx.callCount, 0);

	xpkClose(xpk);
}

TEST(traverse_match_stop_early) {
	xpkObject xpk = xpkOpen("test_13_match_stop.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	for (int i = 0; i < 10; i++) {
		char path[64];
		sprintf(path, "file%d.txt", i);
		xpkPathAppendData(xpk, path, "Data", 4, 6);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_13_match_stop.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	TraverseContext ctx = { 0, 0 };
	int result = xpkEachMatch(xpk, "*.txt", &ctx, stopEarlyCallback);
	ASSERT_EQ(result, 2);
	ASSERT_EQ(ctx.callCount, 2);

	xpkClose(xpk);
}

TEST(traverse_match_empty) {
	xpkObject xpk = xpkOpen("test_13_match_empty.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_13_match_empty.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	TraverseContext ctx = { 0, 0 };
	int result = xpkEachMatch(xpk, "*.txt", &ctx, eachMatchCallback);
	ASSERT_EQ(result, 0);
	ASSERT_EQ(ctx.callCount, 0);

	xpkClose(xpk);
}

TEST(traverse_info_access) {
	xpkObject xpk = xpkOpen("test_13_traverse_info.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	char data1[100] = "Data 1";
	char data2[200] = "Data 2 with more content";
	char data3[50] = "Data 3";

	xpkAppendData(xpk, data1, strlen(data1), 6);
	xpkAppendData(xpk, data2, strlen(data2), 6);
	xpkAppendData(xpk, data3, strlen(data3), 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_13_traverse_info.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	int callCount = 0;
	uint32_t sizes[3] = { 0 };

	void infoCallback(void* ctx, uint32_t pos, void* info) {
		sizes[callCount] = xpkInfoSize((xpkObject)ctx, pos);
		callCount++;
	}

	xpkEach(xpk, xpk, infoCallback);
	ASSERT_EQ(callCount, 3);
	ASSERT_EQ(sizes[0], strlen(data1));
	ASSERT_EQ(sizes[1], strlen(data2));
	ASSERT_EQ(sizes[2], strlen(data3));

	xpkClose(xpk);
}

TEST(traverse_order_preservation) {
	xpkObject xpk = xpkOpen("test_13_traverse_order.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	char files[5][20] = { "First", "Second", "Third", "Fourth", "Fifth" };
	uint32_t sizes[5] = { 5, 6, 5, 6, 5 };

	for (int i = 0; i < 5; i++) {
		xpkAppendData(xpk, files[i], sizes[i], 6);
	}

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_13_traverse_order.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	int callCount = 0;
	char data[5][20] = { 0 };

	void orderCallback(void* ctx, uint32_t pos, void* info) {
		uint32_t outSize = 0;
		void* extracted = xpkExtractData((xpkObject)ctx, pos, &outSize);
		memcpy(data[callCount], extracted, outSize);
		xpkFree(extracted);
		callCount++;
	}

	xpkEach(xpk, xpk, orderCallback);
	ASSERT_EQ(callCount, 5);

	for (int i = 0; i < 5; i++) {
		ASSERT_EQ(memcmp(data[i], files[i], sizes[i]), 0);
	}

	xpkClose(xpk);
}

TEST(traverse_match_case_sensitive_linux) {
	xpkObject xpk = xpkOpen("test_13_match_case.xpk", 0, 0);
	ASSERT_NOT_NULL(xpk);

	ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_LINUX), 0);

	xpkPathAppendData(xpk, "File1.txt", "Data 1", 6, 6);
	xpkPathAppendData(xpk, "file1.txt", "Data 2", 6, 6);
	xpkPathAppendData(xpk, "FILE1.TXT", "Data 3", 6, 6);

	ASSERT_EQ(xpkSave(xpk), 0);
	xpkClose(xpk);

	xpk = xpkOpen("test_13_match_case.xpk", 0, 1);
	ASSERT_NOT_NULL(xpk);

	TraverseContext ctx = { 0, 0 };
	int result = xpkEachMatch(xpk, "file1.txt", &ctx, eachMatchCallback);
	ASSERT_EQ(result, 1);
	ASSERT_EQ(ctx.callCount, 1);

	xpkClose(xpk);
}

int main(void) {
	xrtInit();

	printf("=================================================\n");
	printf("  xPack Ver7 - Traverse Operations Test\n");
	printf("=================================================\n\n");

	printf("[Basic Traverse Tests]\n");
	RUN_TEST(traverse_basic);
	RUN_TEST(traverse_empty);
	RUN_TEST(traverse_single);
	RUN_TEST(traverse_stop_early);
	printf("\n");

	printf("[Match Pattern Tests]\n");
	RUN_TEST(traverse_match_star);
	RUN_TEST(traverse_match_question);
	RUN_TEST(traverse_match_extension);
	RUN_TEST(traverse_match_prefix);
	RUN_TEST(traverse_match_complex);
	RUN_TEST(traverse_match_no_matches);
	RUN_TEST(traverse_match_empty);
	printf("\n");

	printf("[Match Control Tests]\n");
	RUN_TEST(traverse_match_stop_early);
	printf("\n");

	printf("[Info Access Tests]\n");
	RUN_TEST(traverse_info_access);
	RUN_TEST(traverse_order_preservation);
	printf("\n");

	printf("[Case Sensitivity Tests]\n");
	RUN_TEST(traverse_match_case_sensitive_linux);
	printf("\n");

	printf("=================================================\n");
	printf("  Results: %d passed, %d failed\n", tests_passed, tests_failed);
	printf("=================================================\n");

	return tests_failed > 0 ? 1 : 0;
}
