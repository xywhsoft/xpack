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
#define ASSERT_GT(a, b) ASSERT((a) > (b))
#define ASSERT_LT(a, b) ASSERT((a) < (b))
#define ASSERT_GE(a, b) ASSERT((a) >= (b))
#define ASSERT_LE(a, b) ASSERT((a) <= (b))

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

TEST(core_initialization) {
    xpkObject xpk = xpkOpen("test_core_init.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkType(xpk), XPK_TYPE_CORE);
    ASSERT_EQ(xpkCount(xpk), 0);
    xpkClose(xpk);
}

TEST(core_create_empty_package) {
    xpkObject xpk = xpkOpen("test_core_empty.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 0);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_core_empty.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 0);
    xpkClose(xpk);
}

TEST(core_add_file) {
    xpkObject xpk = xpkOpen("test_core_add_file.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    char testData[256];
    memset(testData, 'A', sizeof(testData));
    strcpy(testData, "Test file content");

    xpkAppendData(xpk, testData, sizeof(testData), 6);
    ASSERT_EQ(xpkCount(xpk), 1);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_core_add_file.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 1);
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
// 固实压缩测试
// ============================================================================

TEST(solid_create_basic) {
    xpkObject xpk = xpkOpen("test_solid_basic.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkSolidModeSet(xpk, 1), 0);
    ASSERT_EQ(xpkSolidMode(xpk), 1);

    const char* data = "Solid compression test data";
    uint32_t pos = xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
    ASSERT_EQ(pos, 0);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_solid_basic.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkSolidMode(xpk), 1);

    uint32_t outSize = 0;
    void* extracted = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(extracted);
    ASSERT_EQ(outSize, strlen(data));
    ASSERT_EQ(memcmp(extracted, data, outSize), 0);
    xpkFree(extracted);

    xpkClose(xpk);
}

TEST(solid_add_multiple_files) {
    xpkObject xpk = xpkOpen("test_solid_multi_files.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkSolidModeSet(xpk, 1), 0);

    for (int i = 0; i < 5; i++) {
        char data[128];
        sprintf(data, "File %d content with repeated data", i);
        uint32_t pos = xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
        ASSERT_EQ(pos, i);
    }

    ASSERT_EQ(xpkCount(xpk), 5);
    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_solid_multi_files.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);
    ASSERT_EQ(xpkCount(xpk), 5);

    for (int i = 0; i < 5; i++) {
        char expected[128];
        sprintf(expected, "File %d content with repeated data", i);

        uint32_t outSize = 0;
        void* extracted = xpkExtractData(xpk, i, &outSize);
        ASSERT_NOT_NULL(extracted);
        ASSERT_EQ(outSize, strlen(expected));
        ASSERT_EQ(memcmp(extracted, expected, outSize), 0);
        xpkFree(extracted);
    }

    xpkClose(xpk);
}

TEST(solid_compression_ratio_vs_normal) {
    char dataFiles[5][2048];

    for (int i = 0; i < 5; i++) {
        memset(dataFiles[i], 'A' + (i % 3), sizeof(dataFiles[i]));
        strcpy(dataFiles[i] + 2000, "Common suffix string");
    }

    xpkObject xpkNormal = xpkOpen("test_ratio_normal.xpk", 0, 0);
    ASSERT_NOT_NULL(xpkNormal);

    for (int i = 0; i < 5; i++) {
        xpkAppendData(xpkNormal, dataFiles[i], sizeof(dataFiles[i]), 6);
    }
    ASSERT_EQ(xpkSave(xpkNormal), 0);

    uint32_t normalSize = 0;
    for (uint32_t i = 0; i < xpkCount(xpkNormal); i++) {
        normalSize += xpkInfoPacked(xpkNormal, i);
    }
    xpkClose(xpkNormal);

    xpkObject xpkSolid = xpkOpen("test_ratio_solid.xpk", 0, 0);
    ASSERT_NOT_NULL(xpkSolid);
    ASSERT_EQ(xpkSolidModeSet(xpkSolid, 1), 0);

    for (int i = 0; i < 5; i++) {
        xpkAppendData(xpkSolid, dataFiles[i], sizeof(dataFiles[i]), 6);
    }
    ASSERT_EQ(xpkSave(xpkSolid), 0);

    uint32_t solidOffset = 0;
    uint32_t solidSize = 0;
    ASSERT_EQ(xpkSolidBlockInfo(xpkSolid, &solidOffset, &solidSize), 0);
    xpkClose(xpkSolid);

    ASSERT_GT(normalSize, solidSize);
    ASSERT_GT(solidSize, 0);
}

TEST(solid_empty_files) {
    xpkObject xpk = xpkOpen("test_solid_empty.xpk", 0, 0);
    ASSERT_NOT_NULL(xpk);

    ASSERT_EQ(xpkSolidModeSet(xpk, 1), 0);

    uint32_t pos1 = xpkAppendData(xpk, NULL, 0, 6);
    ASSERT_EQ(pos1, 0);

    uint32_t pos2 = xpkAppendData(xpk, "", 0, 6);
    ASSERT_EQ(pos2, 1);

    const char* data = "Non-empty file";
    uint32_t pos3 = xpkAppendData(xpk, data, (uint32_t)strlen(data), 6);
    ASSERT_EQ(pos3, 2);

    ASSERT_EQ(xpkSave(xpk), 0);
    xpkClose(xpk);

    xpk = xpkOpen("test_solid_empty.xpk", 0, 1);
    ASSERT_NOT_NULL(xpk);

    uint32_t outSize = 0;
    void* extracted = xpkExtractData(xpk, 0, &outSize);
    ASSERT_NOT_NULL(extracted);
    ASSERT_EQ(outSize, 0);
    xpkFree(extracted);

    extracted = xpkExtractData(xpk, 1, &outSize);
    ASSERT_NOT_NULL(extracted);
    ASSERT_EQ(outSize, 0);
    xpkFree(extracted);

    extracted = xpkExtractData(xpk, 2, &outSize);
    ASSERT_NOT_NULL(extracted);
    ASSERT_EQ(outSize, strlen(data));
    ASSERT_EQ(memcmp(extracted, data, outSize), 0);
    xpkFree(extracted);

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
    RUN_TEST(core_initialization);
    RUN_TEST(core_create_empty_package);
    RUN_TEST(core_add_file);
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
    
    printf("[Solid Compression Tests]\n");
    RUN_TEST(solid_create_basic);
    RUN_TEST(solid_add_multiple_files);
    RUN_TEST(solid_compression_ratio_vs_normal);
    RUN_TEST(solid_empty_files);
    printf("\n");

    printf("[Utility Tests]\n");
    RUN_TEST(stat_and_verify);
    printf("\n");
    
    printf("=================================================\n");
    printf("  Results: %d passed, %d failed\n", tests_passed, tests_failed);
    printf("=================================================\n");
    
    return tests_failed > 0 ? 1 : 0;
}
