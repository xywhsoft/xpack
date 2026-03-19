/*
 * xPack Ver7 - Core 模式测试
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xpack.h"

static int g_testsPassed = 0;
static int g_testsFailed = 0;

#define TEST(name) printf("\n[TEST] %s\n", name)
#define PASS() do { g_testsPassed++; printf("  PASS\n"); } while(0)
#define FAIL(msg) do { g_testsFailed++; printf("  FAIL: %s\n", msg); } while(0)

static void generateTestData(void* data, uint64_t size, uint64_t seed)
{
    uint8_t* ptr = (uint8_t*)data;
    uint64_t rng = seed;
    for (uint64_t i = 0; i < size; i++) {
        rng = rng * 1103515245 + 12345;
        ptr[i] = (uint8_t)((rng >> 16) & 0xFF);
    }
}

static int verifyTestData(const void* data, uint64_t size, uint64_t seed)
{
    const uint8_t* ptr = (const uint8_t*)data;
    uint64_t rng = seed;
    for (uint64_t i = 0; i < size; i++) {
        rng = rng * 1103515245 + 12345;
        if (ptr[i] != (uint8_t)((rng >> 16) & 0xFF)) {
            return 0;
        }
    }
    return 1;
}

static void cleanupTestFiles(const char* basePath)
{
    char path[512];
    snprintf(path, sizeof(path), "%s.xpk", basePath);
    xrtFileDelete(path);
    for (int i = 1; i <= 10; i++) {
        snprintf(path, sizeof(path), "%s.%03d.xpk", basePath, i);
        xrtFileDelete(path);
    }
}

static void testEmptyPackage(void)
{
    TEST("EmptyPackage");
    
    cleanupTestFiles("test_empty");
    
    xpkObject xpk = xpkOpen("test_empty", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    if (xpkCount(xpk) != 0) {
        xpkClose(xpk);
        FAIL("New package should have 0 files");
        return;
    }
    
    xpkSave(xpk);
    xpkClose(xpk);
    
    xpk = xpkOpen("test_empty", 0, 1);
    if (!xpk) {
        FAIL("Failed to reopen empty package");
        return;
    }
    
    if (xpkCount(xpk) != 0) {
        xpkClose(xpk);
        FAIL("Reopened package should have 0 files");
        return;
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_empty");
    PASS();
}

static void testAppendExtractData(void)
{
    TEST("AppendExtractData");
    
    cleanupTestFiles("test_append");
    
    xpkObject xpk = xpkOpen("test_append", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    uint8_t writeData[1024];
    generateTestData(writeData, sizeof(writeData), 12345);
    
    uint32_t pos = xpkAppendData(xpk, writeData, sizeof(writeData), 7);
    if (pos == (uint32_t)-1) {
        xpkClose(xpk);
        FAIL("Failed to append data");
        return;
    }
    
    printf("    Appended at pos %u\n", pos);
    
    if (xpkCount(xpk) != 1) {
        xpkClose(xpk);
        FAIL("Count should be 1");
        return;
    }
    
    uint64_t size = 0;
    uint8_t* readData = (uint8_t*)xpkExtractData(xpk, 0, &size);
    if (!readData) {
        xpkClose(xpk);
        FAIL("Failed to extract data");
        return;
    }
    
    if (size != sizeof(writeData)) {
        free(readData);
        xpkClose(xpk);
        FAIL("Size mismatch");
        return;
    }
    
    if (memcmp(writeData, readData, sizeof(writeData)) != 0) {
        free(readData);
        xpkClose(xpk);
        FAIL("Data mismatch");
        return;
    }
    
    free(readData);
    xpkClose(xpk);
    cleanupTestFiles("test_append");
    PASS();
}

static void testMultipleFiles(void)
{
    TEST("MultipleFiles (10 files)");
    
    cleanupTestFiles("test_multi");
    
    xpkObject xpk = xpkOpen("test_multi", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    uint8_t writeData[256];
    
    for (int i = 0; i < 10; i++) {
        generateTestData(writeData, sizeof(writeData), 10000 + i);
        uint32_t pos = xpkAppendData(xpk, writeData, sizeof(writeData), 7);
        if (pos != (uint32_t)i) {
            xpkClose(xpk);
            FAIL("Append returned wrong position");
            return;
        }
    }
    
    if (xpkCount(xpk) != 10) {
        xpkClose(xpk);
        FAIL("Count should be 10");
        return;
    }
    
    for (int i = 0; i < 10; i++) {
        uint8_t* readData = (uint8_t*)xpkExtractData(xpk, (uint32_t)i, NULL);
        if (!readData) {
            xpkClose(xpk);
            FAIL("Failed to extract file");
            return;
        }
        
        generateTestData(writeData, sizeof(writeData), 10000 + i);
        if (memcmp(writeData, readData, sizeof(writeData)) != 0) {
            free(readData);
            xpkClose(xpk);
            FAIL("Data mismatch");
            return;
        }
        free(readData);
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_multi");
    PASS();
}

static void testCompressLevels(void)
{
    TEST("CompressLevels (0-15)");
    
    cleanupTestFiles("test_levels");
    
    uint8_t writeData[4096];
    generateTestData(writeData, sizeof(writeData), 99999);
    
    for (int level = 0; level <= 15; level++) {
        char name[64];
        snprintf(name, sizeof(name), "test_levels_%d", level);
        cleanupTestFiles(name);
        
        xpkObject xpk = xpkOpen(name, 0, 0);
        if (!xpk) {
            FAIL("Failed to create package");
            return;
        }
        
        uint32_t pos = xpkAppendData(xpk, writeData, sizeof(writeData), level);
        if (pos == (uint32_t)-1) {
            xpkClose(xpk);
            FAIL("Append failed");
            return;
        }
        
        uint64_t packedSize = xpkInfoPacked(xpk, pos);
        uint64_t originalSize = xpkInfoSize(xpk, pos);
        double ratio = (double)packedSize / (double)originalSize * 100.0;
        
        printf("    Level %2d: %llu -> %llu (%.1f%%)\n", 
               level, (unsigned long long)originalSize, (unsigned long long)packedSize, ratio);
        
        uint8_t* readData = (uint8_t*)xpkExtractData(xpk, pos, NULL);
        if (!readData) {
            xpkClose(xpk);
            FAIL("Extract failed");
            return;
        }
        
        if (memcmp(writeData, readData, sizeof(writeData)) != 0) {
            free(readData);
            xpkClose(xpk);
            FAIL("Data mismatch");
            return;
        }
        
        free(readData);
        xpkClose(xpk);
        cleanupTestFiles(name);
    }
    
    PASS();
}

static void testSaveAndReopen(void)
{
    TEST("SaveAndReopen");
    
    cleanupTestFiles("test_save");
    
    xpkObject xpk = xpkOpen("test_save", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    uint8_t writeData[512];
    for (int i = 0; i < 5; i++) {
        generateTestData(writeData, sizeof(writeData), 50000 + i);
        xpkAppendData(xpk, writeData, sizeof(writeData), 7);
    }
    
    xpkSave(xpk);
    xpkClose(xpk);
    
    xpk = xpkOpen("test_save", 0, 1);
    if (!xpk) {
        FAIL("Failed to reopen package");
        return;
    }
    
    if (xpkCount(xpk) != 5) {
        xpkClose(xpk);
        FAIL("File count mismatch after reopen");
        return;
    }
    
    for (int i = 0; i < 5; i++) {
        generateTestData(writeData, sizeof(writeData), 50000 + i);
        uint8_t* readData = (uint8_t*)xpkExtractData(xpk, (uint32_t)i, NULL);
        if (!readData) {
            xpkClose(xpk);
            FAIL("Failed to extract after reopen");
            return;
        }
        
        if (memcmp(writeData, readData, sizeof(writeData)) != 0) {
            free(readData);
            xpkClose(xpk);
            FAIL("Data mismatch after reopen");
            return;
        }
        free(readData);
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_save");
    PASS();
}

static void testRemove(void)
{
    TEST("Remove");
    
    cleanupTestFiles("test_remove");
    
    xpkObject xpk = xpkOpen("test_remove", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    uint8_t writeData[256];
    for (int i = 0; i < 3; i++) {
        generateTestData(writeData, sizeof(writeData), 30000 + i);
        xpkAppendData(xpk, writeData, sizeof(writeData), 7);
    }
    
    if (xpkRemove(xpk, 1) != 0) {
        xpkClose(xpk);
        FAIL("Failed to remove file");
        return;
    }
    
    if (!xpkInfoDeleted(xpk, 1)) {
        xpkClose(xpk);
        FAIL("File should be marked as deleted");
        return;
    }
    
    uint8_t* data = (uint8_t*)xpkExtractData(xpk, 1, NULL);
    if (data) {
        free(data);
        xpkClose(xpk);
        FAIL("Should not extract deleted file");
        return;
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_remove");
    PASS();
}

static void testUpdate(void)
{
    TEST("Update");
    
    cleanupTestFiles("test_update");
    
    xpkObject xpk = xpkOpen("test_update", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    uint8_t writeData[256];
    generateTestData(writeData, sizeof(writeData), 11111);
    xpkAppendData(xpk, writeData, sizeof(writeData), 7);
    
    generateTestData(writeData, sizeof(writeData), 22222);
    xpkAppendData(xpk, writeData, sizeof(writeData), 7);
    
    if (!xpkInfoDeleted(xpk, 0)) {
        uint8_t newData[128];
        generateTestData(newData, sizeof(newData), 33333);
        
        if (xpkUpdateData(xpk, 0, newData, sizeof(newData), 7) != 0) {
            xpkClose(xpk);
            FAIL("Failed to update file");
            return;
        }
        
        if (!xpkInfoDeleted(xpk, 0)) {
            xpkClose(xpk);
            FAIL("Old file should be marked deleted");
            return;
        }
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_update");
    PASS();
}

static void testFileInfo(void)
{
    TEST("FileInfo");
    
    cleanupTestFiles("test_info");
    
    xpkObject xpk = xpkOpen("test_info", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    uint8_t writeData[1024];
    generateTestData(writeData, sizeof(writeData), 77777);
    uint32_t pos = xpkAppendData(xpk, writeData, sizeof(writeData), 7);
    
    if (xpkInfoSize(xpk, pos) != sizeof(writeData)) {
        xpkClose(xpk);
        FAIL("InfoSize mismatch");
        return;
    }
    
    uint64_t packed = xpkInfoPacked(xpk, pos);
    if (packed == 0 || packed > sizeof(writeData)) {
        xpkClose(xpk);
        FAIL("InfoPacked invalid");
        return;
    }
    
    uint32_t hash = xpkInfoHash(xpk, pos);
    if (hash == 0) {
        xpkClose(xpk);
        FAIL("InfoHash should not be 0");
        return;
    }
    
    if (xpkInfoLevel(xpk, pos) != 7) {
        xpkClose(xpk);
        FAIL("InfoLevel should be 7");
        return;
    }
    
    xpkInfoTypeSet(xpk, pos, XPK_FTYPE_BINARY);
    if (xpkInfoType(xpk, pos) != XPK_FTYPE_BINARY) {
        xpkClose(xpk);
        FAIL("InfoType mismatch");
        return;
    }
    
    xpkFileInfo* info = (xpkFileInfo*)xpkInfo(xpk, pos);
    if (!info) {
        xpkClose(xpk);
        FAIL("Info returned NULL");
        return;
    }
    
    printf("    Size: %llu, Packed: %llu, Hash: 0x%08X\n",
           (unsigned long long)info->fileSize, 
           (unsigned long long)info->dataSize,
           info->fileHash);
    
    xpkClose(xpk);
    cleanupTestFiles("test_info");
    PASS();
}

static void testLargeFile(void)
{
    TEST("LargeFile (1MB)");
    
    cleanupTestFiles("test_large");
    
    xpkObject xpk = xpkOpen("test_large", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    uint64_t size = 1024 * 1024;
    uint8_t* writeData = (uint8_t*)malloc((size_t)size);
    if (!writeData) {
        xpkClose(xpk);
        FAIL("Memory allocation failed");
        return;
    }
    
    generateTestData(writeData, size, 88888);
    uint32_t pos = xpkAppendData(xpk, writeData, size, 7);
    
    if (pos == (uint32_t)-1) {
        free(writeData);
        xpkClose(xpk);
        FAIL("Failed to append large file");
        return;
    }
    
    uint64_t packed = xpkInfoPacked(xpk, pos);
    printf("    1MB -> %llu bytes (%.1f%%)\n", 
           (unsigned long long)packed, 
           (double)packed / (double)size * 100.0);
    
    uint8_t* readData = (uint8_t*)xpkExtractData(xpk, pos, NULL);
    if (!readData) {
        free(writeData);
        xpkClose(xpk);
        FAIL("Failed to extract large file");
        return;
    }
    
    if (memcmp(writeData, readData, (size_t)size) != 0) {
        free(writeData);
        free(readData);
        xpkClose(xpk);
        FAIL("Large file data mismatch");
        return;
    }
    
    free(writeData);
    free(readData);
    xpkClose(xpk);
    cleanupTestFiles("test_large");
    PASS();
}

static void testStatGet(void)
{
    TEST("StatGet");
    
    cleanupTestFiles("test_stat");
    
    xpkObject xpk = xpkOpen("test_stat", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    uint8_t writeData[1024];
    for (int i = 0; i < 5; i++) {
        generateTestData(writeData, sizeof(writeData), 60000 + i);
        xpkAppendData(xpk, writeData, sizeof(writeData), 7);
    }
    
    xpkStat stat;
    if (xpkStatGet(xpk, &stat) != 0) {
        xpkClose(xpk);
        FAIL("Failed to get stat");
        return;
    }
    
    printf("    Files: %u, Total: %llu, Packed: %llu, Ratio: %.2f%%\n",
           stat.fileCount,
           (unsigned long long)stat.totalSize,
           (unsigned long long)stat.packedSize,
           stat.ratio * 100.0);
    
    if (stat.fileCount != 5) {
        xpkClose(xpk);
        FAIL("File count mismatch");
        return;
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_stat");
    PASS();
}

static void testVerify(void)
{
    TEST("Verify");
    
    cleanupTestFiles("test_verify");
    
    xpkObject xpk = xpkOpen("test_verify", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    uint8_t writeData[256];
    for (int i = 0; i < 3; i++) {
        generateTestData(writeData, sizeof(writeData), 70000 + i);
        xpkAppendData(xpk, writeData, sizeof(writeData), 7);
    }
    
    if (xpkVerify(xpk, 0) != 0) {
        xpkClose(xpk);
        FAIL("Verify file 0 failed");
        return;
    }
    
    if (xpkVerifyAll(xpk) != 0) {
        xpkClose(xpk);
        FAIL("VerifyAll failed");
        return;
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_verify");
    PASS();
}

static void testVolumeMode(void)
{
    TEST("VolumeMode (64KB volume, 3 files)");
    
    cleanupTestFiles("test_volcore");
    
    xpkObject xpk = xpkOpen("test_volcore", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    xpkVolumeSizeSet(xpk, 64 * 1024);
    xpkVolumeModeSet(xpk, 1);
    
    uint8_t writeData[32 * 1024];
    for (int i = 0; i < 3; i++) {
        generateTestData(writeData, sizeof(writeData), 80000 + i);
        uint32_t pos = xpkAppendData(xpk, writeData, sizeof(writeData), 7);
        if (pos == (uint32_t)-1) {
            xpkClose(xpk);
            FAIL("Failed to append in volume mode");
            return;
        }
    }
    
    xpkSave(xpk);
    printf("    Volume count: %d\n", xpkVolumeCount(xpk));
    
    xpkClose(xpk);
    
    xpk = xpkOpen("test_volcore", 0, 1);
    if (!xpk) {
        FAIL("Failed to reopen volume package");
        return;
    }
    
    if (xpkCount(xpk) != 3) {
        xpkClose(xpk);
        FAIL("File count mismatch after reopen");
        return;
    }
    
    for (int i = 0; i < 3; i++) {
        generateTestData(writeData, sizeof(writeData), 80000 + i);
        uint8_t* readData = (uint8_t*)xpkExtractData(xpk, (uint32_t)i, NULL);
        if (!readData) {
            xpkClose(xpk);
            FAIL("Failed to extract in volume mode");
            return;
        }
        
        if (memcmp(writeData, readData, sizeof(writeData)) != 0) {
            free(readData);
            xpkClose(xpk);
            FAIL("Data mismatch in volume mode");
            return;
        }
        free(readData);
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_volcore");
    PASS();
}

static void testRebuild(void)
{
    TEST("Rebuild");
    
    cleanupTestFiles("test_rebuild");
    
    xpkObject xpk = xpkOpen("test_rebuild", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    uint8_t writeData[1024];
    for (int i = 0; i < 5; i++) {
        generateTestData(writeData, sizeof(writeData), 90000 + i);
        xpkAppendData(xpk, writeData, sizeof(writeData), 7);
    }
    
    xpkRemove(xpk, 1);
    xpkRemove(xpk, 3);
    
    xpkStat stat1;
    xpkStatGet(xpk, &stat1);
    printf("    Before rebuild: %u files in list, %u active (2 deleted)\n", xpkCount(xpk), stat1.fileCount);
    
    xpkSave(xpk);
    
    if (xpkRebuild(xpk) != 0) {
        xpkClose(xpk);
        FAIL("Rebuild failed");
        return;
    }
    
    xpkStat stat2;
    xpkStatGet(xpk, &stat2);
    printf("    After rebuild: %u active files\n", stat2.fileCount);
    
    if (stat2.fileCount != 3) {
        xpkClose(xpk);
        FAIL("Active file count should be 3 after rebuild");
        return;
    }
    
    for (int i = 0; i < 3; i++) {
        int originalIndex = (i == 0) ? 0 : (i == 1) ? 2 : 4;
        generateTestData(writeData, sizeof(writeData), 90000 + originalIndex);
        
        uint8_t* readData = (uint8_t*)xpkExtractData(xpk, (uint32_t)i, NULL);
        if (!readData) {
            printf("    Extract failed at pos %d (original seed %d)\n", i, 90000 + originalIndex);
            printf("    Last error: %s\n", xpkLastErrorMsg());
            xpkClose(xpk);
            FAIL("Failed to extract after rebuild");
            return;
        }
        
        if (memcmp(writeData, readData, sizeof(writeData)) != 0) {
            free(readData);
            xpkClose(xpk);
            FAIL("Data mismatch after rebuild");
            return;
        }
        free(readData);
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_rebuild");
    PASS();
}

static void testMeta(void)
{
    TEST("Meta");
    
    cleanupTestFiles("test_meta");
    
    xpkObject xpk = xpkOpen("test_meta", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    uint32_t size = 0;
    void* data = xpkMetaGet(xpk, &size);
    if (data || size != 0) {
        if (data) free(data);
        xpkClose(xpk);
        FAIL("New package should have no meta");
        return;
    }
    
    const char* testData = "Hello, xPack Meta!";
    uint32_t testDataLen = (uint32_t)strlen(testData) + 1;
    
    if (xpkMetaSet(xpk, testData, testDataLen, 7) != 0) {
        xpkClose(xpk);
        FAIL("Failed to set meta");
        return;
    }
    
    data = xpkMetaGet(xpk, &size);
    if (!data) {
        xpkClose(xpk);
        FAIL("Failed to get meta after set");
        return;
    }
    
    if (size != testDataLen || strcmp((const char*)data, testData) != 0) {
        free(data);
        xpkClose(xpk);
        FAIL("Meta data mismatch");
        return;
    }
    
    free(data);
    
    xpkSave(xpk);
    xpkClose(xpk);
    
    xpk = xpkOpen("test_meta", 0, 1);
    if (!xpk) {
        FAIL("Failed to reopen package");
        return;
    }
    
    data = xpkMetaGet(xpk, &size);
    if (!data) {
        xpkClose(xpk);
        FAIL("Failed to get meta after reopen");
        return;
    }
    
    if (size != testDataLen || strcmp((const char*)data, testData) != 0) {
        free(data);
        xpkClose(xpk);
        FAIL("Meta data mismatch after reopen");
        return;
    }
    
    free(data);
    xpkClose(xpk);
    cleanupTestFiles("test_meta");
    PASS();
}

static void testMetaLarge(void)
{
    TEST("MetaLarge (64KB)");
    
    cleanupTestFiles("test_metalarge");
    
    xpkObject xpk = xpkOpen("test_metalarge", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    uint32_t size = 64 * 1024;
    uint8_t* testData = (uint8_t*)malloc(size);
    if (!testData) {
        xpkClose(xpk);
        FAIL("Memory allocation failed");
        return;
    }
    
    generateTestData(testData, size, 55555);
    
    if (xpkMetaSet(xpk, testData, size, 7) != 0) {
        free(testData);
        xpkClose(xpk);
        FAIL("Failed to set large meta");
        return;
    }
    
    uint32_t retSize = 0;
    uint8_t* data = (uint8_t*)xpkMetaGet(xpk, &retSize);
    if (!data) {
        free(testData);
        xpkClose(xpk);
        FAIL("Failed to get large meta");
        return;
    }
    
    if (retSize != size || memcmp(data, testData, size) != 0) {
        free(data);
        free(testData);
        xpkClose(xpk);
        FAIL("Large meta data mismatch");
        return;
    }
    
    free(data);
    free(testData);
    
    xpkSave(xpk);
    xpkClose(xpk);
    cleanupTestFiles("test_metalarge");
    PASS();
}

static void testMetaClear(void)
{
    TEST("MetaClear");
    
    cleanupTestFiles("test_metaclear");
    
    xpkObject xpk = xpkOpen("test_metaclear", 0, 0);
    if (!xpk) {
        FAIL("Failed to create package");
        return;
    }
    
    const char* testData = "Test data";
    if (xpkMetaSet(xpk, testData, (uint32_t)strlen(testData) + 1, 7) != 0) {
        xpkClose(xpk);
        FAIL("Failed to set meta");
        return;
    }
    
    uint32_t size = 0;
    void* data = xpkMetaGet(xpk, &size);
    if (!data || size == 0) {
        xpkClose(xpk);
        FAIL("Meta should exist");
        return;
    }
    free(data);
    
    if (xpkMetaSet(xpk, NULL, 0, 7) != 0) {
        xpkClose(xpk);
        FAIL("Failed to clear meta");
        return;
    }
    
    data = xpkMetaGet(xpk, &size);
    if (data || size != 0) {
        if (data) free(data);
        xpkClose(xpk);
        FAIL("Meta should be cleared");
        return;
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_metaclear");
    PASS();
}

int main(void)
{
    xrtInit();
    
    printf("================================================\n");
    printf(" xPack Ver7 - Core Mode Test\n");
    printf("================================================\n");
    
    testEmptyPackage();
    testAppendExtractData();
    testMultipleFiles();
    testCompressLevels();
    testSaveAndReopen();
    testRemove();
    testUpdate();
    testFileInfo();
    testLargeFile();
    testStatGet();
    testVerify();
    testVolumeMode();
    testRebuild();
    testMeta();
    testMetaLarge();
    testMetaClear();
    
    printf("\n================================================\n");
    printf(" Results: %d tests, %d passed, %d failed\n", 
           g_testsPassed + g_testsFailed, g_testsPassed, g_testsFailed);
    printf("================================================\n");
    
    return (g_testsFailed > 0) ? 1 : 0;
}
