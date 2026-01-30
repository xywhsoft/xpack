/*
 * xPack Ver7 - 测试框架
 *
 * 统一测试宏定义和工具函数
 */

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../lib/xrt/xrt.h"
#include "../src/xpack.h"

// 测试统计
extern int g_tests_passed;
extern int g_tests_failed;

// 测试宏定义
#define TEST(name) void test_##name(void)

#define RUN_TEST(name) do { \
	printf("  Testing %s... ", #name); \
	test_##name(); \
	printf("PASSED\n"); \
	g_tests_passed++; \
} while(0)

#define ASSERT(cond) do { \
	if (!(cond)) { \
		printf("FAILED\n"); \
		printf("    Assertion failed: %s\n", #cond); \
		printf("    At line %d\n", __LINE__); \
		g_tests_failed++; \
		return; \
	} \
} while(0)

#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_NE(a, b) ASSERT((a) != (b))
#define ASSERT_LT(a, b) ASSERT((a) < (b))
#define ASSERT_LE(a, b) ASSERT((a) <= (b))
#define ASSERT_GT(a, b) ASSERT((a) > (b))
#define ASSERT_GE(a, b) ASSERT((a) >= (b))
#define ASSERT_NULL(a) ASSERT((a) == NULL)
#define ASSERT_NOT_NULL(a) ASSERT((a) != NULL)
#define ASSERT_STR_EQ(a, b) ASSERT(strcmp(a, b) == 0)
#define ASSERT_STR_NE(a, b) ASSERT(strcmp(a, b) != 0)
#define ASSERT_STR_CONTAINS(str, substr) ASSERT(strstr(str, substr) != NULL)

// 测试分类枚举
typedef enum {
	TEST_CATEGORY_CORE = 1,
	TEST_CATEGORY_INDEX = 2,
	TEST_CATEGORY_PATH = 3,
	TEST_CATEGORY_COMPRESSION = 4,
	TEST_CATEGORY_SOLID = 5,
	TEST_CATEGORY_ERROR = 6,
	TEST_CATEGORY_UTILS = 7,
	TEST_CATEGORY_BATCH = 8,
	TEST_CATEGORY_TRAVERSE = 9,
	TEST_CATEGORY_STATS = 10,
	TEST_CATEGORY_VERIFY = 11,
	TEST_CATEGORY_REBUILD = 12,
	TEST_CATEGORY_PROPERTIES = 13,
	TEST_CATEGORY_FILETYPE = 14,
	TEST_CATEGORY_CYCLE = 15,
	TEST_CATEGORY_MULTI = 16,
	TEST_CATEGORY_EDGE = 17,
	TEST_CATEGORY_CONCURRENT = 18,
	TEST_CATEGORY_RECOVERY = 19,
	TEST_CATEGORY_PLATFORM = 20,
	TEST_CATEGORY_MEMORY = 21,
	TEST_CATEGORY_INTEGRATION = 22,
	TEST_CATEGORY_PERFORMANCE = 23,
	TEST_CATEGORY_RATIO = 24,
	TEST_CATEGORY_REGRESSION = 25
} TestCategory;

// 测试信息结构
typedef struct {
	const char* name;
	void (*func)(void);
	TestCategory category;
	const char* description;
} TestInfo;

// 测试运行器接口
typedef struct {
	const char* name;
	const char* version;
	int test_count;
	int passed;
	int failed;
	double duration;
} TestRunner;

// 测试函数原型
void test_print_header(const char* title);
void test_print_footer(int passed, int failed);
void test_print_separator(void);
void test_print_category(const char* name);
const char* test_category_name(TestCategory category);

#endif /* TEST_FRAMEWORK_H */
