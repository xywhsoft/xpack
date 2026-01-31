/*
 * xPack Ver7 - 完整测试运行器
 * 
 * 运行所有 33 个测试模块并生成报告
 */

#include "test_framework.h"

// ============================================================================
// 测试套件全局变量
// ============================================================================

TestSuite g_test_suite;
TestCategory g_current_category = CAT_CORE;

// ============================================================================
// 测试套件实现
// ============================================================================

void test_suite_init(const char* name) {
    memset(&g_test_suite, 0, sizeof(TestSuite));
    g_test_suite.name = name;
    g_test_suite.start_time = time(NULL);
    for (int i = 0; i < MAX_CATEGORIES; i++) {
        g_test_suite.stats.cat_counts[i] = 0;
        g_test_suite.stats.cat_passed[i] = 0;
        g_test_suite.stats.cat_failed[i] = 0;
    }
}

int test_suite_register(const char* name, void (*func)(void), TestCategory category, const char* description) {
    if (g_test_suite.count >= MAX_TESTS) {
        printf("Error: Maximum tests exceeded\n");
        return -1;
    }
    
    g_test_suite.tests[g_test_suite.count].name = name;
    g_test_suite.tests[g_test_suite.count].func = func;
    g_test_suite.tests[g_test_suite.count].category = category;
    g_test_suite.tests[g_test_suite.count].description = description;
    g_test_suite.count++;
    g_test_suite.stats.cat_counts[category]++;
    
    return 0;
}

void test_suite_run_all(void) {
    printf("\n");
    printf("=================================================\n");
    printf("  Running All Tests\n");
    printf("=================================================\n\n");
    
    for (int i = 0; i < g_test_suite.count; i++) {
        Test* test = &g_test_suite.tests[i];
        g_current_category = test->category;
        
        printf("[%s] %s\n", test_category_name(test->category), test->description);
        printf("  Testing %s... ", test->name);
        fflush(stdout);
        
        int failed_before = g_test_suite.stats.failed;
        
        test->func();
        
        int failed_after = g_test_suite.stats.failed;
        if (failed_after > failed_before) {
            printf("FAILED\n");
        } else {
            g_test_suite.stats.passed++;
            g_test_suite.stats.cat_passed[test->category]++;
            printf("PASSED\n");
        }
        fflush(stdout);
        
        printf("\n");
    }
}

void test_suite_print_report(void) {
    time_t end_time = time(NULL);
    double elapsed = difftime(end_time, g_test_suite.start_time);
    
    printf("\n");
    printf("=================================================\n");
    printf("  Test Report\n");
    printf("=================================================\n\n");
    
    printf("Summary:\n");
    printf("  Total Tests: %d\n", g_test_suite.stats.total);
    printf("  Passed:      %d\n", g_test_suite.stats.passed);
    printf("  Failed:      %d\n", g_test_suite.stats.failed);
    printf("  Duration:    %.1f seconds\n", elapsed);
    printf("\n");
    
    printf("Results by Category:\n");
    for (int i = 1; i <= CAT_VOLUME; i++) {
        if (g_test_suite.stats.cat_counts[i] > 0) {
            printf("  %-20s: %2d passed, %2d failed (total: %2d)\n",
                   test_category_name((TestCategory)i),
                   g_test_suite.stats.cat_passed[i],
                   g_test_suite.stats.cat_failed[i],
                   g_test_suite.stats.cat_counts[i]);
        }
    }
    printf("\n");
    
    if (g_test_suite.stats.failed > 0) {
        printf("FAILED - Some tests did not pass!\n");
    } else {
        printf("SUCCESS - All tests passed!\n");
    }
    printf("=================================================\n");
}

void test_suite_save_report(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        printf("Error: Cannot open report file: %s\n", filename);
        return;
    }
    
    time_t end_time = time(NULL);
    double elapsed = difftime(end_time, g_test_suite.start_time);
    
    fprintf(fp, "xPack Ver7 - Test Report\n");
    fprintf(fp, "========================================\n\n");
    fprintf(fp, "Generated: %s", ctime(&end_time));
    fprintf(fp, "Duration: %.1f seconds\n\n", elapsed);
    
    fprintf(fp, "Summary:\n");
    fprintf(fp, "  Total Tests: %d\n", g_test_suite.stats.total);
    fprintf(fp, "  Passed:      %d\n", g_test_suite.stats.passed);
    fprintf(fp, "  Failed:      %d\n", g_test_suite.stats.failed);
    fprintf(fp, "\n");
    
    fprintf(fp, "Results by Category:\n");
    for (int i = 1; i <= CAT_VOLUME; i++) {
        if (g_test_suite.stats.cat_counts[i] > 0) {
            fprintf(fp, "  %-20s: %2d passed, %2d failed (total: %2d)\n",
                    test_category_name((TestCategory)i),
                    g_test_suite.stats.cat_passed[i],
                    g_test_suite.stats.cat_failed[i],
                    g_test_suite.stats.cat_counts[i]);
        }
    }
    
    fclose(fp);
    printf("Report saved to: %s\n", filename);
}

const char* test_category_name(TestCategory category) {
    static const char* names[] = {
        "Unknown",
        "Core",
        "Index",
        "Path",
        "Compression",
        "Solid",
        "Error",
        "Utils",
        "Batch",
        "Traverse",
        "Stats",
        "Verify",
        "Rebuild",
        "Properties",
        "FileType",
        "Cycle",
        "Multiple",
        "Edge",
        "Concurrent",
        "Recovery",
        "Platform",
        "Memory",
        "Integration",
        "Performance",
        "Ratio",
        "Regression",
        "Volume"
    };
    
    if (category >= 1 && category <= 26) {
        return names[category];
    }
    return names[0];
}

// ============================================================================
// 包含所有测试模块 (01-33)
// ============================================================================

#include "01_core_basic.h"
#include "02_core_operations.h"
#include "03_core_edge_cases.h"
#include "04_index_operations.h"
#include "05_path_operations.h"
#include "06_path_case_sensitivity.h"
#include "07_compression_data_patterns.h"
#include "08_compression_accuracy.h"
#include "09_compression_large_files.h"
#include "10_solid_compression.h"
#include "11_error_handling.h"
#include "12_batch_operations.h"
#include "13_traverse_operations.h"
#include "14_statistics.h"
#include "15_verify_operations.h"
#include "16_rebuild_operations.h"
#include "17_package_properties.h"
#include "18_file_type.h"
#include "19_save_load_cycles.h"
#include "20_multiple_packages.h"
#include "21_edge_large_files.h"
#include "22_edge_many_files.h"
#include "23_concurrent_access.h"
#include "24_corruption_recovery.h"
#include "25_cross_platform.h"
#include "26_memory_management.h"
#include "27_integration_real_world.h"
#include "28_performance_benchmark.h"
#include "29_compression_ratio.h"
#include "30_regression_tests.h"
#include "31_volume_basic.h"
#include "32_volume_single_file.h"
#include "33_volume_cross.h"

// ============================================================================
// 主函数
// ============================================================================

int main(int argc, char* argv[]) {
    xrtInit();
    
    test_suite_init("xPack Ver7 Complete Test Suite");
    
    printf("=================================================\n");
    printf("  xPack Ver7 - Complete Test Suite (01-33)\n");
    printf("=================================================\n");
    
    printf("\n");
    printf("Registering Tests...\n");
    printf("=================================================\n");
    
    // 注册所有测试模块
    register_01_core_basic_tests();
    printf("  [OK] Core basic tests (01)\n");
    
    register_02_core_operations_tests();
    printf("  [OK] Core operations tests (02)\n");
    
    register_03_core_edge_cases_tests();
    printf("  [OK] Core edge cases tests (03)\n");
    
    register_04_index_operations_tests();
    printf("  [OK] Index operations tests (04)\n");
    
    register_05_path_operations_tests();
    printf("  [OK] Path operations tests (05)\n");
    
    register_06_path_case_sensitivity_tests();
    printf("  [OK] Path case sensitivity tests (06)\n");
    
    register_07_compression_data_patterns_tests();
    printf("  [OK] Compression data patterns tests (07)\n");
    
    register_08_compression_accuracy_tests();
    printf("  [OK] Compression accuracy tests (08)\n");
    
    register_09_compression_large_files_tests();
    printf("  [OK] Compression large files tests (09)\n");
    
    register_10_solid_compression_tests();
    printf("  [OK] Solid compression tests (10)\n");
    
    register_11_error_handling_tests();
    printf("  [OK] Error handling tests (11)\n");
    
    register_12_batch_operations_tests();
    printf("  [OK] Batch operations tests (12)\n");
    
    register_13_traverse_operations_tests();
    printf("  [OK] Traverse operations tests (13)\n");
    
    register_14_statistics_tests();
    printf("  [OK] Statistics tests (14)\n");
    
    register_15_verify_operations_tests();
    printf("  [OK] Verify operations tests (15)\n");
    
    register_16_rebuild_operations_tests();
    printf("  [OK] Rebuild operations tests (16)\n");
    
    register_17_package_properties_tests();
    printf("  [OK] Package properties tests (17)\n");
    
    register_18_file_type_tests();
    printf("  [OK] File type tests (18)\n");
    
    register_19_save_load_cycles_tests();
    printf("  [OK] Save/Load cycles tests (19)\n");
    
    register_20_multiple_packages_tests();
    printf("  [OK] Multiple packages tests (20)\n");
    
    register_21_edge_large_files_tests();
    printf("  [OK] Edge large files tests (21)\n");
    
    register_22_edge_many_files_tests();
    printf("  [OK] Edge many files tests (22)\n");
    
    register_23_concurrent_access_tests();
    printf("  [OK] Concurrent access tests (23)\n");
    
    register_24_corruption_recovery_tests();
    printf("  [OK] Corruption recovery tests (24)\n");
    
    register_25_cross_platform_tests();
    printf("  [OK] Cross platform tests (25)\n");
    
    register_26_memory_management_tests();
    printf("  [OK] Memory management tests (26)\n");
    
    register_27_integration_real_world_tests();
    printf("  [OK] Integration real world tests (27)\n");
    
    register_28_performance_benchmark_tests();
    printf("  [OK] Performance benchmark tests (28)\n");
    
    register_29_compression_ratio_tests();
    printf("  [OK] Compression ratio tests (29)\n");
    
    register_30_regression_tests();
    printf("  [OK] Regression tests (30)\n");
    
    register_31_volume_basic_tests();
    printf("  [OK] Volume basic tests (31)\n");
    
    register_32_volume_single_file_tests();
    printf("  [OK] Volume single file tests (32)\n");
    
    register_33_volume_cross_tests();
    printf("  [OK] Volume cross tests (33)\n");
    
    g_test_suite.stats.total = g_test_suite.count;
    
    printf("\n  Total tests registered: %d\n", g_test_suite.count);
    printf("=================================================\n");
    
    test_suite_run_all();
    test_suite_print_report();
    
    test_suite_save_report("test_report.txt");
    
    return (g_test_suite.stats.failed > 0) ? 1 : 0;
}
