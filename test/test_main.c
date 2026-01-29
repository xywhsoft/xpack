/*
 * xPack Ver7 - 测试主程序
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "xpack.h"
#include <xrt/xrt.h>

// 测试计数
static int tests_passed = 0;
static int tests_failed = 0;

// 测试宏
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

// ============================================================================
// Core 模式测试
// ============================================================================

TEST(core_create_empty) {
    xpkObject xpk = xpkOpen("test_core_empty.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 0);
    ASSERT_EQ(xpkType(xpk), XPK_TYPE_CORE);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);
    
    // 重新打开验证
    xpk = xpkOpen("test_core_empty.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 0);
    xpkClose(xpk);
}

TEST(core_append_data) {
    xpkObject xpk = xpkOpen("test_core_data.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);
    
    const char* data = "Hello xPack Ver7!";
    uint32_t pos = xpkAppendData(xpk, data, (uint32_t)strlen(data), XPK_COMP_DEFAULT);
    ASSERT_EQ(pos, 0);
    ASSERT_EQ(xpkCount(xpk), 1);
    
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);
    
    // 重新打开验证
    xpk = xpkOpen("test_core_data.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 1);
    
    uint32_t outSize = 0;
    void* extracted = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(extracted);
    ASSERT_EQ(outSize, strlen(data));
    ASSERT_EQ(memcmp(extracted, data, outSize), 0);
    
    xpkFree(extracted);
    xpkClose(xpk);
}

TEST(core_multiple_files) {
    xpkObject xpk = xpkOpen("test_core_multi.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);
    
    // 添加多个数据
    for (int i = 0; i < 10; i++) {
        char data[64];
        snprintf(data, sizeof(data), "Data block %d", i);
        uint32_t pos = xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
        ASSERT_EQ(pos, i);
    }
    
    ASSERT_EQ(xpkCount(xpk), 10);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);
    
    // 重新打开验证
    xpk = xpkOpen("test_core_multi.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 10);
    
    for (int i = 0; i < 10; i++) {
        char expected[64];
        snprintf(expected, sizeof(expected), "Data block %d", i);
        
        uint32_t outSize = 0;
        void* extracted = xpkExtractData(xpk, i, &outSize);
        ASSERT_NOT_NULL(extracted);
        ASSERT_EQ(outSize, strlen(expected));
        ASSERT_EQ(memcmp(extracted, expected, outSize), 0);
        xpkFree(extracted);
    }
    
    xpkClose(xpk);
}

// ============================================================================
// 压缩级别测试
// ============================================================================

TEST(compression_levels) {
    uint8_t testData[4096];
    for (int i = 0; i < 4096; i++) {
        testData[i] = (uint8_t)(i % 256);
    }
    
    printf("\n");
    
    for (int level = 0; level <= 15; level++) {
        char filename[64];
        snprintf(filename, sizeof(filename), "test_level_%d.xpk", level);
        
        xpkObject xpk = xpkOpen(filename, 0, 0);
        ASSERT_NOT_NULL(xpk);
        
        uint32_t pos = xpkAppendData(xpk, testData, sizeof(testData), level);
        ASSERT_EQ(pos, 0);
        
        uint32_t packedSize = xpkInfoPacked(xpk, 0);
        printf("    Level %2d: %u -> %u (%.1f%%)\n", 
               level, (unsigned)sizeof(testData), packedSize,
               100.0 * packedSize / sizeof(testData));
        
        ASSERT_EQ(xpkSave(xpk), 0);
        xpkClose(xpk);
        
        // 验证
        xpk = xpkOpen(filename, 0, 1);
        ASSERT_NOT_NULL(xpk);
        
        uint32_t outSize = 0;
        void* extracted = xpkExtractData(xpk, 0, &outSize);
        ASSERT_NOT_NULL(extracted);
        ASSERT_EQ(outSize, sizeof(testData));
        ASSERT_EQ(memcmp(extracted, testData, outSize), 0);
        
        xpkFree(extracted);
        xpkClose(xpk);
    }
    printf("  ");
}

// ============================================================================
// Index 模式测试
// ============================================================================

TEST(index_mode) {
    xpkObject xpk = xpkOpen("test_index.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);
    
    ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_INDEX), 0);
    ASSERT_EQ(xpkType(xpk), XPK_TYPE_INDEX);
    
    // 添加带索引的数据
    xpkFileInfoIndex* info1 = xpkIndexAppendData(xpk, 100, "data100", 7, 6);
    ASSERT_NOT_NULL(info1);
    ASSERT_EQ(info1->fileIndex, 100);
    
    xpkFileInfoIndex* info2 = xpkIndexAppendData(xpk, 200, "data200", 7, 6);
    ASSERT_NOT_NULL(info2);
    
    xpkFileInfoIndex* info3 = xpkIndexAppendData(xpk, 50, "data50", 6, 6);
    ASSERT_NOT_NULL(info3);
    
    ASSERT_EQ(xpkCount(xpk), 3);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);
    
    // 重新打开验证
    xpk = xpkOpen("test_index.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 3);
    
    // 按索引查找
    uint32_t pos = xpkIndexFind(xpk, 200);
    ASSERT_NE(pos, UINT32_MAX);
    
    uint32_t outSize = 0;
    void* data = xpkIndexExtractData(xpk, 200, &outSize);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(outSize, 7);
    ASSERT_EQ(memcmp(data, "data200", 7), 0);
    xpkFree(data);
    
    // 验证其他索引
    data = xpkIndexExtractData(xpk, 100, &outSize);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(memcmp(data, "data100", 7), 0);
    xpkFree(data);
    
    data = xpkIndexExtractData(xpk, 50, &outSize);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(memcmp(data, "data50", 6), 0);
    xpkFree(data);
    
    xpkClose(xpk);
}

// ============================================================================
// 路径模式测试
// ============================================================================

TEST(path_mode_win32) {
    xpkObject xpk = xpkOpen("test_path_win32.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);
    
    ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_WIN32), 0);
    ASSERT_EQ(xpkType(xpk), XPK_TYPE_WIN32);
    
    // 添加带路径的数据
    void* info1 = xpkPathAppendData(xpk, "images/logo.png", "PNG DATA", 8, 0);
    ASSERT_NOT_NULL(info1);
    
    void* info2 = xpkPathAppendData(xpk, "config/settings.json", "{}", 2, 6);
    ASSERT_NOT_NULL(info2);
    
    void* info3 = xpkPathAppendData(xpk, "readme.txt", "README", 6, 6);
    ASSERT_NOT_NULL(info3);
    
    ASSERT_EQ(xpkCount(xpk), 3);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);
    
    // 重新打开验证
    xpk = xpkOpen("test_path_win32.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 3);
    
    // 测试不区分大小写查找
    ASSERT_EQ(xpkPathExists(xpk, "images/logo.png"), 1);
    ASSERT_EQ(xpkPathExists(xpk, "IMAGES/LOGO.PNG"), 1);
    ASSERT_EQ(xpkPathExists(xpk, "Images/Logo.Png"), 1);
    
    // 提取数据
    uint32_t outSize = 0;
    void* data = xpkPathExtractData(xpk, "CONFIG/SETTINGS.JSON", &outSize);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(outSize, 2);
    ASSERT_EQ(memcmp(data, "{}", 2), 0);
    xpkFree(data);
    
    xpkClose(xpk);
}

TEST(path_mode_linux) {
    xpkObject xpk = xpkOpen("test_path_linux.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);
    
    ASSERT_EQ(xpkTypeSet(xpk, XPK_TYPE_LINUX), 0);
    ASSERT_EQ(xpkType(xpk), XPK_TYPE_LINUX);
    
    // 添加带路径的数据
    void* info1 = xpkPathAppendData(xpk, "test/file.txt", "data1", 5, 6);
    ASSERT_NOT_NULL(info1);
    
    void* info2 = xpkPathAppendData(xpk, "test/File.txt", "data2", 5, 6);
    ASSERT_NOT_NULL(info2);  // Linux 区分大小写，应该成功
    
    ASSERT_EQ(xpkCount(xpk), 2);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);
    
    // 重新打开验证
    xpk = xpkOpen("test_path_linux.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    
    // Linux 区分大小写
    ASSERT_EQ(xpkPathExists(xpk, "test/file.txt"), 1);
    ASSERT_EQ(xpkPathExists(xpk, "test/File.txt"), 1);
    ASSERT_EQ(xpkPathExists(xpk, "TEST/FILE.TXT"), 0);  // 不存在
    
    xpkClose(xpk);
}

// ============================================================================
// 统计和校验测试
// ============================================================================

TEST(stat_and_verify) {
    xpkObject xpk = xpkOpen("test_stat.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);
    
    // 添加数据
    char data1[1000];
    char data2[2000];
    memset(data1, 'A', sizeof(data1));
    memset(data2, 'B', sizeof(data2));
    
    xpkAppendData(xpk, data1, sizeof(data1), 6);
    xpkAppendData(xpk, data2, sizeof(data2), 6);
    
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);
    
    // 重新打开验证
    xpk = xpkOpen("test_stat.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    
    // 获取统计
    xpkStat stat;
    ASSERT_EQ(xpkStatGet(xpk, &stat), 0);
    ASSERT_EQ(stat.fileCount, 2);
    ASSERT_EQ(stat.totalSize, 3000);
    
    // 校验
    ASSERT_EQ(xpkVerify(xpk, 0), 0);
    ASSERT_EQ(xpkVerify(xpk, 1), 0);
    ASSERT_EQ(xpkVerifyAll(xpk), 0);
    
    xpkClose(xpk);
}

// ============================================================================
// 主函数
// ============================================================================

int main(void) {
    // 初始化 xrt 库
    xrtInit();
    
    printf("=================================================\n");
    printf("  xPack Ver7 Test Suite\n");
    printf("=================================================\n\n");
    
    printf("[Core Mode Tests]\n");
    RUN_TEST(core_create_empty);
    RUN_TEST(core_append_data);
    RUN_TEST(core_multiple_files);
    printf("\n");
    
    printf("[Compression Tests]\n");
    RUN_TEST(compression_levels);
    printf("\n");
    
    printf("[Index Mode Tests]\n");
    RUN_TEST(index_mode);
    printf("\n");
    
    printf("[Path Mode Tests]\n");
    RUN_TEST(path_mode_win32);
    RUN_TEST(path_mode_linux);
    printf("\n");
    
    printf("[Utility Tests]\n");
    RUN_TEST(stat_and_verify);
    printf("\n");
    
    printf("=================================================\n");
    printf("  Results: %d passed, %d failed\n", tests_passed, tests_failed);
    printf("=================================================\n");
    
    return tests_failed > 0 ? 1 : 0;
}
