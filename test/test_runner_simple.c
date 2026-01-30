/*
 * xPack Ver7 - 简化版测试运行器
 *
 * 统一运行所有测试用例
 */

#include "test_framework.h"
#include <time.h>

// 测试套件结构
typedef struct {
	const char* name;
	void (*run)(void);
} TestSuite;

// ============================================================================
// Core 模式测试
// ============================================================================
void test_core_run(void) {
	printf("[Core Mode Tests]\n");
	RUN_TEST(core_create_empty);
	RUN_TEST(core_append_data);
	RUN_TEST(core_multiple_files);
}

// ============================================================================
// 压缩测试
// ============================================================================
void test_compression_run(void) {
	printf("\n[Compression Tests]\n");
	RUN_TEST(compression_levels);
}

// ============================================================================
// Index 模式测试
// ============================================================================
void test_index_run(void) {
	printf("\n[Index Mode Tests]\n");
	RUN_TEST(index_mode);
}

// ============================================================================
// Path 模式测试
// ============================================================================
void test_path_run(void) {
	printf("\n[Path Mode Tests]\n");
	RUN_TEST(path_mode_win32);
	RUN_TEST(path_mode_linux);
}

// ============================================================================
// 固实压缩测试
// ============================================================================
void test_solid_run(void) {
	printf("\n[Solid Compression Tests]\n");
	RUN_TEST(solid_mode_enable_disable);
	RUN_TEST(solid_mode_multiple_files);
	RUN_TEST(solid_mode_empty_files);
	RUN_TEST(solid_mode_compression_ratio);
	RUN_TEST(solid_mode_block_info);
	RUN_TEST(solid_mode_cannot_set_after_files);
}

// ============================================================================
// 统计和校验测试
// ============================================================================
void test_stats_run(void) {
	printf("\n[Statistics and Verify Tests]\n");
	RUN_TEST(stat_and_verify);
}

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char* argv[]) {
	clock_t start_time = clock();
	
	xrtInit();
	
	test_print_header("Complete Test Suite");
	
	// 重置测试统计
	g_tests_passed = 0;
	g_tests_failed = 0;
	
	// 运行所有测试套件
	test_core_run();
	test_compression_run();
	test_index_run();
	test_path_run();
	test_solid_run();
	test_stats_run();
	
	// 打印结果
	clock_t end_time = clock();
	double duration = (double)(end_time - start_time) / CLOCKS_PER_SEC;
	
	printf("\n");
	test_print_footer(g_tests_passed, g_tests_failed);
	printf("  Total time: %.2f seconds\n", duration);
	printf("=================================================\n");
	
	return g_tests_failed > 0 ? 1 : 0;
}
