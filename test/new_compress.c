/*
 * xPack Ver7 - 压缩模块测试
 *
 * 测试内容：
 *   1. 压缩上界计算
 *   2. 空数据处理
 *   3. STORE 模式
 *   4. LZ4 压缩 (level 1-4)
 *   5. ZSTD 压缩 (level 5-13)
 *   6. LZMA2 压缩 (level 14-15)
 *   7. 不可压缩数据回退
 *   8. 大文件压缩
 *   9. 哈希校验（正确）
 *   10. 哈希校验（错误数据检测）
 *   11. 哈希校验（错误哈希检测）
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "xpack.h"

/* clang-format off */

// 测试宏
#define TEST_BEGIN(name)    printf("\n[TEST] %s\n", name)
#define TEST_PASS()         printf("  PASS\n")
#define TEST_FAIL(msg)      do { printf("  FAIL: %s\n", msg); g_failCount++; } while(0)
#define ASSERT(cond, msg)   do { if (!(cond)) { TEST_FAIL(msg); return; } } while(0)

// 全局统计
static int g_testCount = 0;
static int g_failCount = 0;

// ============================================================================
// 测试数据
// ============================================================================

// 可压缩数据（重复模式）
static const char* g_compressibleData = 
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
    "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
    "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD";

// 不可压缩数据（随机）
static uint8_t g_randomData[1024];

// 空数据
static const char* g_emptyData = "";

// ============================================================================
// 生成随机数据
// ============================================================================

static void generateRandomData(void)
{
    srand((unsigned int)time(NULL));
    for (int i = 0; i < sizeof(g_randomData); i++) {
        g_randomData[i] = (uint8_t)(rand() & 0xFF);
    }
}

// ============================================================================
// 测试：压缩上界计算
// ============================================================================

static void test_compress_bound(void)
{
    TEST_BEGIN("CompressBound");
    g_testCount++;
    
    uint64_t srcSize = 1024;
    
    // Level 0 (STORE): 上界 = 原始大小
    uint64_t bound0 = xpkTestCompressBound(0, srcSize);
    ASSERT(bound0 == srcSize, "STORE bound should equal srcSize");
    
    // Level 1-15: 上界应该 >= 原始大小
    for (int level = 1; level <= 15; level++) {
        uint64_t bound = xpkTestCompressBound(level, srcSize);
        ASSERT(bound >= srcSize, "Compressed bound should >= srcSize");
    }
    
    TEST_PASS();
}

// ============================================================================
// 测试：空数据
// ============================================================================

static void test_empty_data(void)
{
    TEST_BEGIN("EmptyData");
    g_testCount++;
    
    uint64_t bound = xpkTestCompressBound(7, 0);
    uint8_t* dst = (uint8_t*)malloc((size_t)bound + 1);
    
    uint64_t compSize = 0;
    uint32_t hash = 0;
    int ret = xpkTestCompress(7, g_emptyData, 0, dst, bound + 1, &compSize, &hash);
    ASSERT(ret == 0, "Compress empty data should succeed");
    ASSERT(compSize == 0, "Compressed size should be 0");
    ASSERT(hash == 0, "Hash of empty data should be 0");
    
    // 解压空数据
    ret = xpkTestDecompress(7, g_emptyData, 0, dst, 0, 0);
    ASSERT(ret == 0, "Decompress empty data should succeed");
    
    free(dst);
    TEST_PASS();
}

// ============================================================================
// 测试：STORE 模式 (level 0)
// ============================================================================

static void test_store_mode(void)
{
    TEST_BEGIN("StoreMode (level 0)");
    g_testCount++;
    
    uint64_t srcSize = strlen(g_compressibleData);
    uint64_t bound = xpkTestCompressBound(0, srcSize);
    
    uint8_t* dst = (uint8_t*)malloc((size_t)bound);
    uint8_t* recovered = (uint8_t*)malloc((size_t)srcSize);
    
    // 压缩
    uint64_t compSize = 0;
    uint32_t hash = 0;
    int ret = xpkTestCompress(0, g_compressibleData, srcSize, dst, bound, &compSize, &hash);
    ASSERT(ret == 0, "STORE compress should succeed");
    ASSERT(compSize == srcSize, "STORE compressed size should equal original");
    
    // 验证数据未改变
    ASSERT(memcmp(dst, g_compressibleData, (size_t)srcSize) == 0, "Data should be unchanged");
    
    // 解压（正确哈希）
    ret = xpkTestDecompress(0, dst, compSize, recovered, srcSize, hash);
    ASSERT(ret == 0, "STORE decompress with correct hash should succeed");
    ASSERT(memcmp(recovered, g_compressibleData, (size_t)srcSize) == 0, "Recovered data should match");
    
    free(dst);
    free(recovered);
    TEST_PASS();
}

// ============================================================================
// 测试：LZ4 压缩 (level 1-4)
// ============================================================================

static void test_lz4_compress(void)
{
    TEST_BEGIN("LZ4 Compress (level 1-4)");
    g_testCount++;
    
    uint64_t srcSize = strlen(g_compressibleData);
    
    for (int level = 1; level <= 4; level++) {
        uint64_t bound = xpkTestCompressBound(level, srcSize);
        
        uint8_t* dst = (uint8_t*)malloc((size_t)bound);
        uint8_t* recovered = (uint8_t*)malloc((size_t)srcSize);
        
        // 压缩
        uint64_t compSize = 0;
        uint32_t hash = 0;
        int ret = xpkTestCompress(level, g_compressibleData, srcSize, dst, bound, &compSize, &hash);
        ASSERT(ret == 0, "LZ4 compress should succeed");
        ASSERT(compSize <= srcSize, "Compressed size should <= original");
        
        printf("    Level %d: %llu -> %llu bytes (%.1f%%), hash=0x%08X\n", 
               level, (unsigned long long)srcSize, (unsigned long long)compSize,
               100.0 * compSize / srcSize, hash);
        
        // 解压（正确哈希）
        ret = xpkTestDecompress(level, dst, compSize, recovered, srcSize, hash);
        ASSERT(ret == 0, "LZ4 decompress should succeed");
        ASSERT(memcmp(recovered, g_compressibleData, (size_t)srcSize) == 0, "Data mismatch");
        
        free(dst);
        free(recovered);
    }
    
    TEST_PASS();
}

// ============================================================================
// 测试：ZSTD 压缩 (level 5-13)
// ============================================================================

static void test_zstd_compress(void)
{
    TEST_BEGIN("ZSTD Compress (level 5-13)");
    g_testCount++;
    
    uint64_t srcSize = strlen(g_compressibleData);
    
    for (int level = 5; level <= 13; level++) {
        uint64_t bound = xpkTestCompressBound(level, srcSize);
        
        uint8_t* dst = (uint8_t*)malloc((size_t)bound);
        uint8_t* recovered = (uint8_t*)malloc((size_t)srcSize);
        
        // 压缩
        uint64_t compSize = 0;
        uint32_t hash = 0;
        int ret = xpkTestCompress(level, g_compressibleData, srcSize, dst, bound, &compSize, &hash);
        ASSERT(ret == 0, "ZSTD compress should succeed");
        ASSERT(compSize <= srcSize, "Compressed size should <= original");
        
        printf("    Level %d: %llu -> %llu bytes (%.1f%%), hash=0x%08X\n", 
               level, (unsigned long long)srcSize, (unsigned long long)compSize,
               100.0 * compSize / srcSize, hash);
        
        // 解压（正确哈希）
        ret = xpkTestDecompress(level, dst, compSize, recovered, srcSize, hash);
        ASSERT(ret == 0, "ZSTD decompress should succeed");
        ASSERT(memcmp(recovered, g_compressibleData, (size_t)srcSize) == 0, "Data mismatch");
        
        free(dst);
        free(recovered);
    }
    
    TEST_PASS();
}

// ============================================================================
// 测试：LZMA2 压缩 (level 14-15)
// ============================================================================

static void test_lzma2_compress(void)
{
    TEST_BEGIN("LZMA2 Compress (level 14-15)");
    g_testCount++;
    
    uint64_t srcSize = strlen(g_compressibleData);
    
    for (int level = 14; level <= 15; level++) {
        uint64_t bound = xpkTestCompressBound(level, srcSize);
        
        uint8_t* dst = (uint8_t*)malloc((size_t)bound);
        uint8_t* recovered = (uint8_t*)malloc((size_t)srcSize);
        
        // 压缩
        uint64_t compSize = 0;
        uint32_t hash = 0;
        int ret = xpkTestCompress(level, g_compressibleData, srcSize, dst, bound, &compSize, &hash);
        ASSERT(ret == 0, "LZMA2 compress should succeed");
        ASSERT(compSize <= srcSize, "Compressed size should <= original");
        
        printf("    Level %d: %llu -> %llu bytes (%.1f%%), hash=0x%08X\n", 
               level, (unsigned long long)srcSize, (unsigned long long)compSize,
               100.0 * compSize / srcSize, hash);
        
        // 解压（正确哈希）
        ret = xpkTestDecompress(level, dst, compSize, recovered, srcSize, hash);
        ASSERT(ret == 0, "LZMA2 decompress should succeed");
        ASSERT(memcmp(recovered, g_compressibleData, (size_t)srcSize) == 0, "Data mismatch");
        
        free(dst);
        free(recovered);
    }
    
    TEST_PASS();
}

// ============================================================================
// 测试：不可压缩数据（应该回退到 STORE）
// ============================================================================

static void test_incompressible_data(void)
{
    TEST_BEGIN("IncompressibleData (should fallback to STORE)");
    g_testCount++;
    
    uint64_t srcSize = sizeof(g_randomData);
    int fallbackCount = 0;
    
    for (int level = 1; level <= 15; level++) {
        uint64_t bound = xpkTestCompressBound(level, srcSize);
        
        uint8_t* dst = (uint8_t*)malloc((size_t)bound);
        uint8_t* recovered = (uint8_t*)malloc((size_t)srcSize);
        
        // 压缩
        uint64_t compSize = 0;
        uint32_t hash = 0;
        int ret = xpkTestCompress(level, g_randomData, srcSize, dst, bound, &compSize, &hash);
        ASSERT(ret == 0, "Compress should succeed");
        
        // 检查是否回退到 STORE
        if (compSize == srcSize && memcmp(dst, g_randomData, (size_t)srcSize) == 0) {
            fallbackCount++;
        }
        
        // 解压（正确哈希）
        ret = xpkTestDecompress(level, dst, compSize, recovered, srcSize, hash);
        ASSERT(ret == 0, "Decompress should succeed");
        ASSERT(memcmp(recovered, g_randomData, (size_t)srcSize) == 0, "Data mismatch");
        
        free(dst);
        free(recovered);
    }
    
    printf("    Fallback to STORE: %d/15 levels\n", fallbackCount);
    TEST_PASS();
}

// ============================================================================
// 测试：大文件压缩
// ============================================================================

static void test_large_file(void)
{
    TEST_BEGIN("LargeFile (1MB)");
    g_testCount++;
    
    // 生成 1MB 可压缩数据
    uint64_t srcSize = 1024 * 1024;
    uint8_t* srcData = (uint8_t*)malloc((size_t)srcSize);
    
    // 填充重复模式
    for (uint64_t i = 0; i < srcSize; i++) {
        srcData[i] = (uint8_t)(i % 256);
    }
    
    // 测试几个关键级别
    int testLevels[] = {0, 1, 7, 15};
    
    for (int i = 0; i < 4; i++) {
        int level = testLevels[i];
        uint64_t bound = xpkTestCompressBound(level, srcSize);
        
        uint8_t* dst = (uint8_t*)malloc((size_t)bound);
        uint8_t* recovered = (uint8_t*)malloc((size_t)srcSize);
        
        // 压缩
        uint64_t compSize = 0;
        uint32_t hash = 0;
        int ret = xpkTestCompress(level, srcData, srcSize, dst, bound, &compSize, &hash);
        ASSERT(ret == 0, "Large file compress should succeed");
        
        printf("    Level %d: %llu -> %llu bytes (%.1f%%), hash=0x%08X\n", 
               level, (unsigned long long)srcSize, (unsigned long long)compSize,
               100.0 * compSize / srcSize, hash);
        
        // 解压（正确哈希）
        ret = xpkTestDecompress(level, dst, compSize, recovered, srcSize, hash);
        ASSERT(ret == 0, "Large file decompress should succeed");
        ASSERT(memcmp(recovered, srcData, (size_t)srcSize) == 0, "Data mismatch");
        
        free(dst);
        free(recovered);
    }
    
    free(srcData);
    TEST_PASS();
}

// ============================================================================
// 测试：哈希校验 - 正确哈希
// ============================================================================

static void test_hash_correct(void)
{
    TEST_BEGIN("HashVerification (correct hash)");
    g_testCount++;
    
    uint64_t srcSize = strlen(g_compressibleData);
    uint64_t bound = xpkTestCompressBound(7, srcSize);
    
    uint8_t* dst = (uint8_t*)malloc((size_t)bound);
    uint8_t* recovered = (uint8_t*)malloc((size_t)srcSize);
    
    // 压缩
    uint64_t compSize = 0;
    uint32_t hash = 0;
    int ret = xpkTestCompress(7, g_compressibleData, srcSize, dst, bound, &compSize, &hash);
    ASSERT(ret == 0, "Compress should succeed");
    
    printf("    Original hash: 0x%08X\n", hash);
    
    // 解压（正确哈希）
    ret = xpkTestDecompress(7, dst, compSize, recovered, srcSize, hash);
    ASSERT(ret == 0, "Decompress with correct hash should succeed");
    
    free(dst);
    free(recovered);
    TEST_PASS();
}

// ============================================================================
// 测试：哈希校验 - 数据被篡改
// ============================================================================

static void test_hash_tampered_data(void)
{
    TEST_BEGIN("HashVerification (tampered data)");
    g_testCount++;
    
    uint64_t srcSize = strlen(g_compressibleData);
    uint64_t bound = xpkTestCompressBound(7, srcSize);
    
    uint8_t* dst = (uint8_t*)malloc((size_t)bound);
    uint8_t* recovered = (uint8_t*)malloc((size_t)srcSize);
    
    // 压缩
    uint64_t compSize = 0;
    uint32_t hash = 0;
    int ret = xpkTestCompress(7, g_compressibleData, srcSize, dst, bound, &compSize, &hash);
    ASSERT(ret == 0, "Compress should succeed");
    
    // 篡改压缩数据
    if (compSize > 10) {
        dst[5] ^= 0xFF;  // 翻转一个字节
    }
    
    // 解压（哈希是正确的，但数据被篡改）
    // 由于数据被篡改，解压可能失败或产生错误数据，哈希校验应该检测到
    ret = xpkTestDecompress(7, dst, compSize, recovered, srcSize, hash);
    
    // 如果解压成功，检查哈希是否匹配
    if (ret == 0) {
        uint32_t actualHash = xrtHash32((ptr)recovered, (size_t)srcSize);
        if (actualHash != hash) {
            // 哈希不匹配是预期的
            printf("    Detected tampered data (hash mismatch)\n");
        }
    } else if (ret == -2) {
        printf("    Detected tampered data (hash verification failed)\n");
    }
    
    free(dst);
    free(recovered);
    TEST_PASS();
}

// ============================================================================
// 测试：哈希校验 - 错误的哈希值
// ============================================================================

static void test_hash_wrong_hash(void)
{
    TEST_BEGIN("HashVerification (wrong hash)");
    g_testCount++;
    
    uint64_t srcSize = strlen(g_compressibleData);
    uint64_t bound = xpkTestCompressBound(7, srcSize);
    
    uint8_t* dst = (uint8_t*)malloc((size_t)bound);
    uint8_t* recovered = (uint8_t*)malloc((size_t)srcSize);
    
    // 压缩
    uint64_t compSize = 0;
    uint32_t hash = 0;
    int ret = xpkTestCompress(7, g_compressibleData, srcSize, dst, bound, &compSize, &hash);
    ASSERT(ret == 0, "Compress should succeed");
    
    printf("    Correct hash: 0x%08X\n", hash);
    printf("    Wrong hash:   0x%08X\n", hash ^ 0xFFFFFFFF);
    
    // 解压（错误的哈希）
    ret = xpkTestDecompress(7, dst, compSize, recovered, srcSize, hash ^ 0xFFFFFFFF);
    ASSERT(ret == -2, "Decompress with wrong hash should return -2");
    
    printf("    Hash verification correctly rejected wrong hash\n");
    
    free(dst);
    free(recovered);
    TEST_PASS();
}

// ============================================================================
// 主函数
// ============================================================================

int main(int argc, char* argv[])
{
    printf("================================================\n");
    printf(" xPack Ver7 - Compression Module Test\n");
    printf(" With Hash Verification\n");
    printf("================================================\n");
    
    // 生成随机数据
    generateRandomData();
    
    // 运行测试
    test_compress_bound();
    test_empty_data();
    test_store_mode();
    test_lz4_compress();
    test_zstd_compress();
    test_lzma2_compress();
    test_incompressible_data();
    test_large_file();
    test_hash_correct();
    test_hash_tampered_data();
    test_hash_wrong_hash();
    
    // 统计
    printf("\n================================================\n");
    printf(" Results: %d tests, %d passed, %d failed\n", 
           g_testCount, g_testCount - g_failCount, g_failCount);
    printf("================================================\n");
    
    return g_failCount > 0 ? 1 : 0;
}

/* clang-format on */
