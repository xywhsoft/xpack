/*
 * xPack Ver7 - 稳定测试程序
 * 
 * 统一测试 xPack 所有功能并生成报告
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <setjmp.h>
#include <xrt/xrt.h>
#include "../src/xpack.h"

#define MAX_TESTS 50
#define MAX_CATEGORIES 11

jmp_buf g_test_jmp;

typedef enum {
    CAT_CORE = 1,
    CAT_COMPRESSION = 2,
    CAT_PATH = 3,
    CAT_SOLID = 4,
    CAT_ERROR = 5,
    CAT_BATCH = 6,
    CAT_STATS = 7,
    CAT_PROPERTIES = 8,
    CAT_MEMORY = 9,
    CAT_VOLUME = 10
} Category;

typedef struct {
    const char* name;
    const char* description;
    Category category;
    void (*func)(void);
} Test;

typedef struct {
    int total;
    int passed;
    int failed;
    int cat_counts[MAX_CATEGORIES];
    int cat_passed[MAX_CATEGORIES];
    int cat_failed[MAX_CATEGORIES];
} Stats;

Stats g_stats;
Category g_current_cat;

#define TEST(name) void test_##name(void)

#define REGISTER_TEST(name, cat, desc) \
    register_test(#name, test_##name, cat, desc)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("\n      Assertion failed: (%s)\n", #cond); \
        printf("      File: %s, Line: %d\n", __FILE__, __LINE__); \
        fflush(stdout); \
        g_stats.failed++; \
        g_stats.cat_failed[g_current_cat]++; \
        longjmp(g_test_jmp, 1); \
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

Test g_tests[MAX_TESTS];
int g_test_count = 0;
time_t g_start_time;
char g_test_prefix[64];

void register_test(const char* name, void (*func)(void), Category cat, const char* desc) {
    if (g_test_count >= MAX_TESTS) return;
    g_tests[g_test_count].name = name;
    g_tests[g_test_count].func = func;
    g_tests[g_test_count].category = cat;
    g_tests[g_test_count].description = desc;
    g_test_count++;
    g_stats.cat_counts[cat]++;
}

const char* cat_name(Category cat) {
    static const char* names[] = {
        "Unknown", "Core", "Compression", "Path", "Solid",
        "Error", "Batch", "Stats", "Properties", "Memory", "Volume"
    };
    if (cat >= 1 && cat <= 10) return names[cat];
    return names[0];
}

void cleanup_test_files(void) {
    for (int i = 0; i < 100; i++) {
        char filename[256];
        sprintf(filename, "%s%d.xpk", g_test_prefix, i);
        xrtFileDelete(filename);
        sprintf(filename, "%s%d.txt", g_test_prefix, i);
        xrtFileDelete(filename);
    }
    
    xrtFileDelete("stable_01_src.txt");
    xrtFileDelete("stable_01_add.xpk");
    xrtFileDelete("stable_01_data.xpk");
    xrtFileDelete("stable_01_init.xpk");
    xrtFileDelete("stable_01_save.xpk");
    xrtFileDelete("stable_02_lz4.xpk");
    xrtFileDelete("stable_02_zstd.xpk");
    xrtFileDelete("stable_02_lzma2.xpk");
    xrtFileDelete("stable_03_solid.xpk");
    xrtFileDelete("stable_04_batch.xpk");
    xrtFileDelete("stable_05_stats.xpk");
    xrtFileDelete("stable_06_props.xpk");
    xrtFileDelete("stable_07_memory.xpk");
    xrtFileDelete("stable_08_vol_mode.xpk");
    xrtFileDelete("stable_08_vol_size.xpk");
    xrtFileDelete("stable_08_vol_split.xpk");
}

int run_test(Test* test) {
    printf("  Testing %s... ", test->name);
    fflush(stdout);
    
    g_current_cat = test->category;
    int failed_before = g_stats.failed;
    
    if (setjmp(g_test_jmp) == 0) {
        test->func();
        
        int failed_after = g_stats.failed;
        if (failed_after > failed_before) {
            return 0;
        }
        
        g_stats.passed++;
        g_stats.cat_passed[test->category]++;
        printf("PASSED\n");
        fflush(stdout);
        return 1;
    } else {
        printf("FAILED\n");
        fflush(stdout);
        return 0;
    }
}

void print_report(void) {
    time_t end_time = time(NULL);
    double elapsed = difftime(end_time, g_start_time);
    
    printf("\n");
    printf("=================================================\n");
    printf("  Test Report\n");
    printf("=================================================\n\n");
    
    printf("Summary:\n");
    printf("  Total Tests: %d\n", g_stats.total);
    printf("  Passed:      %d\n", g_stats.passed);
    printf("  Failed:      %d\n", g_stats.failed);
    printf("  Duration:    %.1f seconds\n", elapsed);
    printf("\n");
    
    printf("Results by Category:\n");
    for (int i = 1; i <= CAT_VOLUME; i++) {
        if (g_stats.cat_counts[i] > 0) {
            printf("  %-20s: %2d passed, %2d failed (total: %2d)\n",
                   cat_name((Category)i),
                   g_stats.cat_passed[i],
                   g_stats.cat_failed[i],
                   g_stats.cat_counts[i]);
        }
    }
    printf("\n");
    
    if (g_stats.failed > 0) {
        printf("FAILED - Some tests did not pass!\n");
    } else {
        printf("SUCCESS - All tests passed!\n");
    }
    printf("=================================================\n");
    
    FILE* fp = fopen("test_report.txt", "w");
    if (fp) {
        fprintf(fp, "xPack Ver7 - Test Report\n");
        fprintf(fp, "========================================\n\n");
        fprintf(fp, "Generated: %s", ctime(&end_time));
        fprintf(fp, "Duration: %.1f seconds\n\n", elapsed);
        
        fprintf(fp, "Summary:\n");
        fprintf(fp, "  Total Tests: %d\n", g_stats.total);
        fprintf(fp, "  Passed:      %d\n", g_stats.passed);
        fprintf(fp, "  Failed:      %d\n", g_stats.failed);
        fprintf(fp, "\n");
        
        fprintf(fp, "Results by Category:\n");
        for (int i = 1; i <= CAT_VOLUME; i++) {
            if (g_stats.cat_counts[i] > 0) {
                fprintf(fp, "  %-20s: %2d passed, %2d failed (total: %2d)\n",
                        cat_name((Category)i),
                        g_stats.cat_passed[i],
                        g_stats.cat_failed[i],
                        g_stats.cat_counts[i]);
            }
        }
        fclose(fp);
        printf("Report saved to: test_report.txt\n");
    }
}

// ============================================================================
// 核心基本功能测试
// ============================================================================

TEST(core_initialization) {
    xpkObject xpk = xpkOpen("stable_01_init.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkType(xpk), XPK_TYPE_CORE);
    ASSERT_EQ(xpkCount(xpk), 0);
    xpkClose(xpk);
}

TEST(core_add_file) {
    char srcPath[256];
    sprintf(srcPath, "stable_01_src.txt");
    FILE* fp = fopen(srcPath, "wb");
    fwrite("Test data", 1, 9, fp);
    fclose(fp);
    
    xpkObject xpk = xpkOpen("stable_01_add.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);
    
    uint32_t pos = xpkAppendFile(xpk, srcPath, 6);
    ASSERT_EQ(pos, 0);
    ASSERT_EQ(xpkCount(xpk), 1);
    
    xpkSave(xpk);
    xpkClose(xpk);
    
    xrtFileDelete(srcPath);
}

TEST(core_add_data) {
    xpkObject xpk = xpkOpen("stable_01_data.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);
    
    char data[] = "Test data from memory";
    uint32_t pos = xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
    ASSERT_EQ(pos, 0);
    ASSERT_EQ(xpkCount(xpk), 1);
    
    xpkSave(xpk);
    xpkClose(xpk);
}

TEST(core_save_load) {
    xpkObject xpk = xpkOpen("stable_01_save.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);
    
    xpkAppendData(xpk, "First", 5, 6);
    xpkAppendData(xpk, "Second", 6, 6);
    
    xpkSave(xpk);
    xpkClose(xpk);
    
    printf("DEBUG: Opening stable_01_save.xpk (readonly=1)...\n");
    xpk = xpkOpen("stable_01_save.xpk", 0, 1);
    if (xpk == NULL) {
        printf("DEBUG: xpkOpen failed with error %d: %s\n", xpkLastError(), xpkLastErrorMsg());
    }
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 2);
    xpkClose(xpk);
}

// ============================================================================
// 压缩算法测试
// ============================================================================

TEST(compression_lz4) {
    xpkObject xpk = xpkOpen("stable_02_lz4.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);
    
    char testData[4096];
    memset(testData, 'X', sizeof(testData));
    xpkAppendData(xpk, testData, sizeof(testData), 1);
    xpkSave(xpk);
    xpkClose(xpk);
    
    xrtSleep(100);
    
    xpk = xpkOpen("stable_02_lz4.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    
    uint32_t packedSize = xpkInfoPacked(xpk, 0);
    ASSERT_GT(packedSize, 0);
    ASSERT_LT(packedSize, sizeof(testData));
    
    uint32_t outSize = 0;
    void* extracted = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(extracted);
    ASSERT_EQ(outSize, sizeof(testData));
    ASSERT_EQ(memcmp(extracted, testData, outSize), 0);
    xpkFree(extracted);
    xpkClose(xpk);
}

TEST(compression_zstd) {
    xpkObject xpk = xpkOpen("stable_02_zstd.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);
    
    char testData[4096];
    for (int i = 0; i < 4096; i++) {
        testData[i] = (char)(i % 256);
    }
    
    xpkAppendData(xpk, testData, sizeof(testData), 6);
    xpkSave(xpk);
    xpkClose(xpk);
    
    xrtSleep(100);
    
    xpk = xpkOpen("stable_02_zstd.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    
    uint32_t packedSize = xpkInfoPacked(xpk, 0);
    ASSERT_GT(packedSize, 0);
    ASSERT_LT(packedSize, sizeof(testData));
    
    uint32_t outSize = 0;
    void* extracted = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(extracted);
    ASSERT_EQ(outSize, sizeof(testData));
    ASSERT_EQ(memcmp(extracted, testData, outSize), 0);
    xpkFree(extracted);
    xpkClose(xpk);
}

TEST(compression_lzma2) {
    xpkObject xpk = xpkOpen("stable_02_lzma2.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);
    
    char testData[8192];
    for (int i = 0; i < 8192; i++) {
        testData[i] = (char)(i % 16);
    }
    
    xpkAppendData(xpk, testData, sizeof(testData), 11);
    xpkSave(xpk);
    xpkClose(xpk);
    
    xrtSleep(100);
    
    xpk = xpkOpen("stable_02_lzma2.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    
    uint32_t packedSize = xpkInfoPacked(xpk, 0);
    ASSERT_GT(packedSize, 0);
    ASSERT_LT(packedSize, sizeof(testData));
    
    uint32_t outSize = 0;
    void* extracted = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(extracted);
    ASSERT_EQ(outSize, sizeof(testData));
    ASSERT_EQ(memcmp(extracted, testData, outSize), 0);
    xpkFree(extracted);
    xpkClose(xpk);
}

// ============================================================================
// 固实压缩测试
// ============================================================================

TEST(solid_compression) {
    xpkObject xpk = xpkOpen("stable_03_solid.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);
    
    xpkSolidModeSet(xpk, 1);
    ASSERT_EQ(xpkSolidMode(xpk), 1);
    
    for (int i = 0; i < 5; i++) {
        char data[64];
        sprintf(data, "Solid file %d", i);
        xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
    }
    
    xpkSave(xpk);
    xpkClose(xpk);
    
    xpk = xpkOpen("stable_03_solid.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkSolidMode(xpk), 1);
    ASSERT_EQ(xpkCount(xpk), 5);
    xpkClose(xpk);
}

// ============================================================================
// 批量操作测试
// ============================================================================

TEST(batch_operations) {
    xpkObject xpk = xpkOpen("stable_04_batch.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);
    
    for (int i = 0; i < 20; i++) {
        char data[64];
        sprintf(data, "Batch file %d", i);
        xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
    }
    
    ASSERT_EQ(xpkCount(xpk), 20);
    xpkSave(xpk);
    xpkClose(xpk);
    
    xrtSleep(100);
    
    xpk = xpkOpen("stable_04_batch.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 20);
    xpkClose(xpk);
}

// ============================================================================
// 统计功能测试
// ============================================================================

TEST(statistics) {
    xpkObject xpk = xpkOpen("stable_05_stats.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);
    
    xpkAppendData(xpk, "Data 1", 6, 6);
    xpkAppendData(xpk, "Data 2", 6, 6);
    xpkAppendData(xpk, "Data 3", 6, 6);
    
    ASSERT_EQ(xpkCount(xpk), 3);
    
    uint32_t totalSize = 0;
    uint32_t totalPacked = 0;
    for (uint32_t i = 0; i < xpkCount(xpk); i++) {
        totalSize += xpkInfoSize(xpk, i);
        totalPacked += xpkInfoPacked(xpk, i);
    }
    
    ASSERT_GT(totalSize, 0);
    ASSERT_GT(totalPacked, 0);
    
    xpkSave(xpk);
    xpkClose(xpk);
}

// ============================================================================
// 包属性测试
// ============================================================================

TEST(package_properties) {
    xpkObject xpk = xpkOpen("stable_06_props.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);
    
    ASSERT_EQ(xpkType(xpk), XPK_TYPE_CORE);
    ASSERT_EQ(xpkCount(xpk), 0);
    ASSERT_EQ(xpkSolidMode(xpk), 0);
    
    xpkAppendData(xpk, "Test", 4, 6);
    
    ASSERT_EQ(xpkCount(xpk), 1);
    ASSERT_EQ(xpkInfoLevel(xpk, 0), 6);
    ASSERT_EQ(xpkInfoSize(xpk, 0), 4);
    ASSERT_GT(xpkInfoPacked(xpk, 0), 0);
    
    xpkSave(xpk);
    xpkClose(xpk);
}

// ============================================================================
// 内存管理测试
// ============================================================================

TEST(memory_management) {
    xpkObject xpk = xpkOpen("stable_07_memory.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);
    
    char largeData[1024 * 100];
    memset(largeData, 'M', sizeof(largeData));
    
    xpkAppendData(xpk, largeData, sizeof(largeData), 6);
    xpkSave(xpk);
    xpkClose(xpk);
    
    xrtSleep(100);
    
    xpk = xpkOpen("stable_07_memory.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    
    uint32_t outSize = 0;
    void* extracted = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(extracted);
    ASSERT_EQ(outSize, sizeof(largeData));
    ASSERT_EQ(memcmp(extracted, largeData, outSize), 0);
    xpkFree(extracted);
    
    xpkClose(xpk);
}

// ============================================================================
// 分卷功能测试
// ============================================================================

TEST(volume_mode_enable_disable) {
    xpkObject xpk = xpkOpen("stable_08_vol_mode.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);
    
    ASSERT_EQ(xpkVolumeMode(xpk), 0);
    
    ASSERT_EQ(xpkVolumeModeSet(xpk, 1), 0);
    ASSERT_EQ(xpkVolumeMode(xpk), 1);
    
    ASSERT_EQ(xpkVolumeModeSet(xpk, 0), 0);
    ASSERT_EQ(xpkVolumeMode(xpk), 0);
    
    xpkSave(xpk);  // 确保更改被保存
    xpkClose(xpk);
    
    // 重新打开验证
    xpk = xpkOpen("stable_08_vol_mode.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkVolumeMode(xpk), 0);  // 应该是禁用状态
    xpkClose(xpk);
}

TEST(volume_size_configuration) {
    xpkObject xpk = xpkOpen("stable_08_vol_size.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);
    
    ASSERT_EQ(xpkVolumeSizeSet(xpk, 1024), 0);
    ASSERT_EQ(xpkVolumeSize(xpk), 1024);
    
    ASSERT_EQ(xpkVolumeSizeSet(xpk, 2048), 0);
    ASSERT_EQ(xpkVolumeSize(xpk), 2048);
    
    ASSERT_EQ(xpkVolumeSizeSet(xpk, 4096), 0);
    ASSERT_EQ(xpkVolumeSize(xpk), 4096);
    
    ASSERT_EQ(xpkVolumeSizeSet(xpk, 0), 0);
    ASSERT_EQ(xpkVolumeSize(xpk), 0);
    
    xpkSave(xpk);  // 确保更改被保存
    xpkClose(xpk);
    
    // 重新打开验证
    xpk = xpkOpen("stable_08_vol_size.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    xpkClose(xpk);
}

TEST(volume_split_mode) {
    xpkObject xpk = xpkOpen("stable_08_vol_split.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);
    
    ASSERT_EQ(xpkVolumeSplitModeSet(xpk, 0), 0);
    ASSERT_EQ(xpkVolumeSplitMode(xpk), 0);
    
    ASSERT_EQ(xpkVolumeSplitModeSet(xpk, 1), 0);
    ASSERT_EQ(xpkVolumeSplitMode(xpk), 1);
    
    ASSERT_EQ(xpkVolumeSplitModeSet(xpk, 0), 0);
    ASSERT_EQ(xpkVolumeSplitMode(xpk), 0);
    
    xpkSave(xpk);  // 确保更改被保存
    xpkClose(xpk);
    
    // 重新打开验证
    xpk = xpkOpen("stable_08_vol_split.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    xpkClose(xpk);
}

// ============================================================================
// 主函数
// ============================================================================

int main(int argc, char* argv[]) {
    xrtInit();
    
    memset(&g_stats, 0, sizeof(Stats));
    g_start_time = time(NULL);
    strcpy(g_test_prefix, "stable_");
    
    printf("=================================================\n");
    printf("  xPack Ver7 - Complete Test Suite\n");
    printf("=================================================\n\n");
    
    printf("Cleaning up old test files...\n");
    cleanup_test_files();
    
    printf("Registering Tests...\n");
    printf("=================================================\n");
    
    REGISTER_TEST(core_initialization, CAT_CORE, "Test basic initialization");
    REGISTER_TEST(core_add_file, CAT_CORE, "Test adding file to package");
    REGISTER_TEST(core_add_data, CAT_CORE, "Test adding data from memory");
    REGISTER_TEST(core_save_load, CAT_CORE, "Test save and load operations");
    
    REGISTER_TEST(compression_lz4, CAT_COMPRESSION, "Test LZ4 compression");
    REGISTER_TEST(compression_zstd, CAT_COMPRESSION, "Test ZSTD compression");
    REGISTER_TEST(compression_lzma2, CAT_COMPRESSION, "Test LZMA2 compression");
    
    REGISTER_TEST(solid_compression, CAT_SOLID, "Test solid compression");
    
    REGISTER_TEST(batch_operations, CAT_BATCH, "Test batch operations");
    
    REGISTER_TEST(statistics, CAT_STATS, "Test statistics");
    
    REGISTER_TEST(package_properties, CAT_PROPERTIES, "Test package properties");
    
    REGISTER_TEST(memory_management, CAT_MEMORY, "Test memory management");
    
    REGISTER_TEST(volume_mode_enable_disable, CAT_VOLUME, "Test enabling and disabling volume mode");
    REGISTER_TEST(volume_size_configuration, CAT_VOLUME, "Test volume size configuration");
    REGISTER_TEST(volume_split_mode, CAT_VOLUME, "Test volume split mode");
    
    g_stats.total = g_test_count;
    
    printf("\n  Total tests registered: %d\n", g_test_count);
    printf("=================================================\n\n");
    
    printf("Running Tests...\n");
    printf("=================================================\n\n");
    
    for (int i = 0; i < g_test_count; i++) {
        printf("[%s] %s\n", cat_name(g_tests[i].category), g_tests[i].description);
        run_test(&g_tests[i]);
        printf("\n");
    }
    
    print_report();
    
    cleanup_test_files();
    
    return (g_stats.failed > 0) ? 1 : 0;
}
