#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#define PATH_SEPARATOR '\\'
#else
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#define PATH_SEPARATOR '/'
#endif

extern int g_tests_passed;
extern int g_tests_failed;
extern int g_tests_total;

void test_set_failed(void);
int test_get_failed(void);
void test_clear_failed(void);

#define TEST(name) void test_##name(void)

#define RUN_TEST(name) do { \
	printf("  Testing %s... ", #name); \
	fflush(stdout); \
	g_tests_total++; \
	test_clear_failed(); \
	test_##name(); \
	if (test_get_failed()) { \
		printf("FAILED\n"); \
		g_tests_failed++; \
	} else { \
		printf("PASSED\n"); \
		g_tests_passed++; \
	} \
} while(0)

#define ASSERT(cond) do { \
	if (!(cond)) { \
		test_set_failed(); \
		printf("    Assertion failed: %s\n", #cond); \
		printf("    At line %d\n", __LINE__); \
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

typedef enum {
	TEST_CATEGORY_BASIC = 1,
	TEST_CATEGORY_COMPRESSION = 2,
	TEST_CATEGORY_EXTRACT = 3,
	TEST_CATEGORY_LIST = 4,
	TEST_CATEGORY_DELETE = 5,
	TEST_CATEGORY_UPDATE = 6,
	TEST_CATEGORY_SOLID = 7,
	TEST_CATEGORY_TYPES = 8,
	TEST_CATEGORY_EDGE = 9,
	TEST_CATEGORY_VOLUME = 10
} TestCategory;

void test_print_header(const char* title);
void test_print_footer(void);
void test_print_separator(void);
void test_print_category(const char* name);
const char* test_category_name(TestCategory category);

int test_run_command(const char* cmd);
int test_file_exists(const char* path);
int test_file_size(const char* path, long* size);
int test_file_compare(const char* file1, const char* file2);
int test_file_read(const char* path, void** buffer, long* size);
void test_file_free(void* buffer);
int test_dir_create(const char* path);
int test_dir_remove(const char* path);
int test_dir_exists(const char* path);
int test_clean_test_files(void);
int test_create_test_file(const char* path, const char* content);
int test_create_test_binary_file(const char* path, const void* data, size_t size);
int test_get_xpkcon_path_copy(char* buffer, size_t size);
void test_set_xpkcon_path(const char* path);
const char* test_get_xpkcon_path(void);

#endif
