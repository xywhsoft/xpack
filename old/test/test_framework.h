/*
 * xPack Ver7 - 统一测试框架
 */

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include "../lib/xrt/xrt.h"
#include "../src/xpack.h"

#define MAX_TESTS 300
#define MAX_CATEGORIES 30

typedef enum {
    CAT_UNKNOWN = 0,
    CAT_CORE = 1,
    CAT_INDEX = 2,
    CAT_PATH = 3,
    CAT_COMPRESSION = 4,
    CAT_SOLID = 5,
    CAT_ERROR = 6,
    CAT_UTILS = 7,
    CAT_BATCH = 8,
    CAT_TRAVERSE = 9,
    CAT_STATS = 10,
    CAT_VERIFY = 11,
    CAT_REBUILD = 12,
    CAT_PROPERTIES = 13,
    CAT_FILETYPE = 14,
    CAT_CYCLE = 15,
    CAT_MULTI = 16,
    CAT_EDGE = 17,
    CAT_CONCURRENT = 18,
    CAT_RECOVERY = 19,
    CAT_PLATFORM = 20,
    CAT_MEMORY = 21,
    CAT_INTEGRATION = 22,
    CAT_PERFORMANCE = 23,
    CAT_RATIO = 24,
    CAT_REGRESSION = 25,
    CAT_VOLUME = 26
} TestCategory;

typedef struct {
    const char* name;
    const char* description;
    TestCategory category;
    void (*func)(void);
} Test;

typedef struct {
    int total;
    int passed;
    int failed;
    int cat_counts[MAX_CATEGORIES];
    int cat_passed[MAX_CATEGORIES];
    int cat_failed[MAX_CATEGORIES];
} TestStats;

typedef struct {
    const char* name;
    Test tests[MAX_TESTS];
    int count;
    TestStats stats;
    time_t start_time;
} TestSuite;

extern TestSuite g_test_suite;
extern TestCategory g_current_category;

#define TEST(name) void test_##name(void)

#define TEST_REGISTER(name, category, description) \
    do { \
        test_suite_register(#name, test_##name, category, description); \
    } while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("      FAILED\n"); \
        printf("      Assertion failed: %s\n", #cond); \
        printf("      File: %s, Line: %d\n", __FILE__, __LINE__); \
        fflush(stdout); \
        g_test_suite.stats.failed++; \
        g_test_suite.stats.cat_failed[g_current_category]++; \
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

void test_suite_init(const char* name);
int test_suite_register(const char* name, void (*func)(void), TestCategory category, const char* description);
void test_suite_run_all(void);
void test_suite_print_report(void);
void test_suite_save_report(const char* filename);
const char* test_category_name(TestCategory category);

#endif /* TEST_FRAMEWORK_H */
