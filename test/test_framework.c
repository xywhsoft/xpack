/*
 * xPack Ver7 - 测试框架实现
 */

#include "test_framework.h"

// 全局测试统计
int g_tests_passed = 0;
int g_tests_failed = 0;

// 测试分类名称映射
const char* test_category_name(TestCategory category) {
	switch (category) {
		case TEST_CATEGORY_CORE: return "Core Operations";
		case TEST_CATEGORY_INDEX: return "Index Operations";
		case TEST_CATEGORY_PATH: return "Path Operations";
		case TEST_CATEGORY_COMPRESSION: return "Compression Tests";
		case TEST_CATEGORY_SOLID: return "Solid Compression";
		case TEST_CATEGORY_ERROR: return "Error Handling";
		case TEST_CATEGORY_UTILS: return "Utility Functions";
		case TEST_CATEGORY_BATCH: return "Batch Operations";
		case TEST_CATEGORY_TRAVERSE: return "Traverse Operations";
		case TEST_CATEGORY_STATS: return "Statistics";
		case TEST_CATEGORY_VERIFY: return "Verify Operations";
		case TEST_CATEGORY_REBUILD: return "Rebuild Operations";
		case TEST_CATEGORY_PROPERTIES: return "Package Properties";
		case TEST_CATEGORY_FILETYPE: return "File Type Tests";
		case TEST_CATEGORY_CYCLE: return "Save/Load Cycles";
		case TEST_CATEGORY_MULTI: return "Multiple Packages";
		case TEST_CATEGORY_EDGE: return "Edge Cases";
		case TEST_CATEGORY_CONCURRENT: return "Concurrent Access";
		case TEST_CATEGORY_RECOVERY: return "Corruption Recovery";
		case TEST_CATEGORY_PLATFORM: return "Cross Platform";
		case TEST_CATEGORY_MEMORY: return "Memory Management";
		case TEST_CATEGORY_INTEGRATION: return "Integration Tests";
		case TEST_CATEGORY_PERFORMANCE: return "Performance Benchmark";
		case TEST_CATEGORY_RATIO: return "Compression Ratio";
		case TEST_CATEGORY_REGRESSION: return "Regression Tests";
		default: return "Unknown Category";
	}
}

// 打印测试标题
void test_print_header(const char* title) {
	printf("=================================================\n");
	printf("  xPack Ver7 - %s\n", title);
	printf("=================================================\n\n");
}

// 打印测试页脚
void test_print_footer(int passed, int failed) {
	printf("=================================================\n");
	printf("  Results: %d passed, %d failed\n", passed, failed);
	printf("=================================================\n");
}

// 打印分隔线
void test_print_separator(void) {
	printf("-------------------------------------------------\n");
}

// 打印测试分类
void test_print_category(const char* name) {
	printf("[%s]\n", name);
}
