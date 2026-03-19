/*
 * xPack Ver7 - 分卷虚拟 I/O 测试
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

static void testOpenClose(void)
{
    TEST("OpenClose");
    
    cleanupTestFiles("test_open");
    
    xpkObject xpk = xpkOpen("test_open", 0, 0);
    if (!xpk) {
        FAIL("Failed to create xpk object");
        return;
    }
    
    xpkHead* head = xpkGetHead(xpk);
    if (!head) {
        xpkClose(xpk);
        FAIL("Failed to get header");
        return;
    }
    
    printf("    fileHead: 0x%08X (expected: 0x%08X)\n", head->fileHead, XPK_VERSION);
    if (head->fileHead != XPK_VERSION) {
        xpkClose(xpk);
        FAIL("Invalid file header");
        return;
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_open");
    PASS();
}

static void testRawWriteRead(void)
{
    TEST("RawWriteRead");
    
    cleanupTestFiles("test_raw");
    
    xpkObject xpk = xpkOpen("test_raw", 0, 0);
    if (!xpk) {
        FAIL("Failed to create xpk object");
        return;
    }
    
    uint8_t writeData[1024];
    generateTestData(writeData, sizeof(writeData), 12345);
    
    if (xpkRawWrite(xpk, 0, sizeof(writeData), writeData) != 0) {
        xpkClose(xpk);
        FAIL("RawWrite failed");
        return;
    }
    
    uint8_t readData[1024];
    memset(readData, 0, sizeof(readData));
    
    if (xpkRawRead(xpk, 0, sizeof(readData), readData) != 0) {
        xpkClose(xpk);
        FAIL("RawRead failed");
        return;
    }
    
    if (memcmp(writeData, readData, sizeof(writeData)) != 0) {
        xpkClose(xpk);
        FAIL("Data mismatch");
        return;
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_raw");
    PASS();
}

static void testRawOffsetWriteRead(void)
{
    TEST("RawOffsetWriteRead");
    
    cleanupTestFiles("test_offset");
    
    xpkObject xpk = xpkOpen("test_offset", 0, 0);
    if (!xpk) {
        FAIL("Failed to create xpk object");
        return;
    }
    
    uint8_t writeData[256];
    generateTestData(writeData, sizeof(writeData), 54321);
    
    uint64_t offset = 4096;
    
    if (xpkRawWrite(xpk, offset, sizeof(writeData), writeData) != 0) {
        xpkClose(xpk);
        FAIL("RawWrite at offset failed");
        return;
    }
    
    uint8_t readData[256];
    memset(readData, 0, sizeof(readData));
    
    if (xpkRawRead(xpk, offset, sizeof(readData), readData) != 0) {
        xpkClose(xpk);
        FAIL("RawRead at offset failed");
        return;
    }
    
    if (memcmp(writeData, readData, sizeof(writeData)) != 0) {
        xpkClose(xpk);
        FAIL("Data mismatch at offset");
        return;
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_offset");
    PASS();
}
static void testRawLargeData(void)
{
    TEST("RawLargeData (1MB)");
    
    cleanupTestFiles("test_large");
    
    xpkObject xpk = xpkOpen("test_large", 0, 0);
    if (!xpk) {
        FAIL("Failed to create xpk object");
        return;
    }
    
    uint64_t size = 1024 * 1024;
    uint8_t* writeData = (uint8_t*)malloc((size_t)size);
    uint8_t* readData = (uint8_t*)malloc((size_t)size);
    
    if (!writeData || !readData) {
        free(writeData);
        free(readData);
        xpkClose(xpk);
        FAIL("Memory allocation failed");
        return;
    }
    
    generateTestData(writeData, size, 99999);
    memset(readData, 0, (size_t)size);
    
    if (xpkRawWrite(xpk, 0, size, writeData) != 0) {
        free(writeData);
        free(readData);
        xpkClose(xpk);
        FAIL("RawWrite large data failed");
        return;
    }
    
    if (xpkRawRead(xpk, 0, size, readData) != 0) {
        free(writeData);
        free(readData);
        xpkClose(xpk);
        FAIL("RawRead large data failed");
        return;
    }
    
    if (memcmp(writeData, readData, (size_t)size) != 0) {
        free(writeData);
        free(readData);
        xpkClose(xpk);
        FAIL("Large data mismatch");
        return;
    }
    
    free(writeData);
    free(readData);
    xpkClose(xpk);
    cleanupTestFiles("test_large");
    PASS();
}

static void testVolumeMapping(void)
{
    TEST("VolumeMapping (64KB volume, 128KB data)");
    
    cleanupTestFiles("test_vol");
    
    xpkObject xpk = xpkOpen("test_vol", 0, 0);
    if (!xpk) {
        FAIL("Failed to create xpk object");
        return;
    }
    
    xpkVolumeSizeSet(xpk, 64 * 1024);
    xpkVolumeModeSet(xpk, 1);
    
    uint64_t dataSize = 128 * 1024;
    uint8_t* writeData = (uint8_t*)malloc((size_t)dataSize);
    uint8_t* readData = (uint8_t*)malloc((size_t)dataSize);
    
    if (!writeData || !readData) {
        free(writeData);
        free(readData);
        xpkClose(xpk);
        FAIL("Memory allocation failed");
        return;
    }
    
    generateTestData(writeData, dataSize, 77777);
    memset(readData, 0, (size_t)dataSize);
    
    if (xpkRawWrite(xpk, 0, dataSize, writeData) != 0) {
        free(writeData);
        free(readData);
        xpkClose(xpk);
        FAIL("Volume write failed");
        return;
    }
    
    if (xpkRawRead(xpk, 0, dataSize, readData) != 0) {
        free(writeData);
        free(readData);
        xpkClose(xpk);
        FAIL("Volume read failed");
        return;
    }
    
    if (memcmp(writeData, readData, (size_t)dataSize) != 0) {
        free(writeData);
        free(readData);
        xpkClose(xpk);
        FAIL("Volume data mismatch");
        return;
    }
    
    printf("    Volume count: %d\n", xpkVolumeCount(xpk));
    
    free(writeData);
    free(readData);
    xpkClose(xpk);
    cleanupTestFiles("test_vol");
    PASS();
}
static void testVolumeCrossBoundary(void)
{
    TEST("VolumeCrossBoundary (32KB offset, 128KB read)");
    
    cleanupTestFiles("test_cross");
    
    xpkObject xpk = xpkOpen("test_cross", 0, 0);
    if (!xpk) {
        FAIL("Failed to create xpk object");
        return;
    }
    
    xpkVolumeSizeSet(xpk, 64 * 1024);
    xpkVolumeModeSet(xpk, 1);
    
    uint64_t offset = 32 * 1024;
    uint64_t dataSize = 128 * 1024;
    uint8_t* writeData = (uint8_t*)malloc((size_t)dataSize);
    uint8_t* readData = (uint8_t*)malloc((size_t)dataSize);
    
    if (!writeData || !readData) {
        free(writeData);
        free(readData);
        xpkClose(xpk);
        FAIL("Memory allocation failed");
        return;
    }
    
    generateTestData(writeData, dataSize, 88888);
    memset(readData, 0, (size_t)dataSize);
    
    if (xpkRawWrite(xpk, offset, dataSize, writeData) != 0) {
        free(writeData);
        free(readData);
        xpkClose(xpk);
        FAIL("Cross-boundary write failed");
        return;
    }
    
    if (xpkRawRead(xpk, offset, dataSize, readData) != 0) {
        free(writeData);
        free(readData);
        xpkClose(xpk);
        FAIL("Cross-boundary read failed");
        return;
    }
    
    if (memcmp(writeData, readData, (size_t)dataSize) != 0) {
        free(writeData);
        free(readData);
        xpkClose(xpk);
        FAIL("Cross-boundary data mismatch");
        return;
    }
    
    printf("    Write offset: %llu, size: %llu\n", (unsigned long long)offset, (unsigned long long)dataSize);
    printf("    Expected: vol0[32K-64K] + vol1[0-64K] + vol2[0-32K]\n");
    printf("    Volume count: %d\n", xpkVolumeCount(xpk));
    
    free(writeData);
    free(readData);
    xpkClose(xpk);
    cleanupTestFiles("test_cross");
    PASS();
}
static void testReadonlyWrite(void)
{
    TEST("ReadonlyWrite (should fail)");
    
    cleanupTestFiles("test_ro");
    
    xpkObject xpk = xpkOpen("test_ro", 0, 0);
    if (!xpk) {
        FAIL("Failed to create xpk object");
        return;
    }
    xpkClose(xpk);
    
    xpk = xpkOpen("test_ro", 0, 1);
    if (!xpk) {
        FAIL("Failed to open readonly xpk object");
        return;
    }
    
    uint8_t data[64];
    memset(data, 0xAA, sizeof(data));
    
    int result = xpkRawWrite(xpk, 0, sizeof(data), data);
    
    xpkClose(xpk);
    cleanupTestFiles("test_ro");
    
    if (result == 0) {
        FAIL("Write should have failed in readonly mode");
        return;
    }
    
    PASS();
}
static void testNullParams(void)
{
    TEST("NullParams");
    
    uint8_t data[64];
    memset(data, 0, sizeof(data));
    
    if (xpkRawRead(NULL, 0, sizeof(data), data) != -1) {
        FAIL("xpkRawRead with NULL xpk should fail");
        return;
    }
    
    if (xpkRawWrite(NULL, 0, sizeof(data), data) != -1) {
        FAIL("xpkRawWrite with NULL xpk should fail");
        return;
    }
    
    cleanupTestFiles("test_null");
    
    xpkObject xpk = xpkOpen("test_null", 0, 0);
    if (!xpk) {
        FAIL("Failed to create xpk object");
        return;
    }
    
    if (xpkRawRead(xpk, 0, sizeof(data), NULL) != -1) {
        xpkClose(xpk);
        FAIL("xpkRawRead with NULL data should fail");
        return;
    }
    
    if (xpkRawWrite(xpk, 0, 0, data) != -1) {
        xpkClose(xpk);
        FAIL("xpkRawWrite with size 0 should fail");
        return;
    }
    
    xpkClose(xpk);
    cleanupTestFiles("test_null");
    PASS();
}
static void testSaveAndReopen(void)
{
    TEST("SaveAndReopen");
    
    cleanupTestFiles("test_reopen");
    
    xpkObject xpk = xpkOpen("test_reopen", 0, 0);
    if (!xpk) {
        FAIL("Failed to create xpk object");
        return;
    }
    
    xpkVolumeSizeSet(xpk, 32 * 1024);
    xpkVolumeModeSet(xpk, 1);
    
    xpkBufferedSet(xpk, 1);
    
    uint8_t writeData[1024];
    generateTestData(writeData, sizeof(writeData), 11111);
    
    uint32_t pos = xpkAppendData(xpk, writeData, sizeof(writeData), 7);
    if (pos == (uint32_t)-1) {
        xpkClose(xpk);
        FAIL("AppendData failed");
        return;
    }
    
    if (xpkSave(xpk) != 0) {
        xpkClose(xpk);
        FAIL("Save failed");
        return;
    }
    
    xpkClose(xpk);
    
    xpk = xpkOpen("test_reopen", 0, 1);
    if (!xpk) {
        FAIL("Failed to reopen xpk object");
        return;
    }
    
    xpkHead* head = xpkGetHead(xpk);
    if (head->volumeSize != 32 * 1024) {
        xpkClose(xpk);
        FAIL("VolumeSize not preserved");
        return;
    }
    
    if (head->volumeMode != 1) {
        xpkClose(xpk);
        FAIL("VolumeMode not preserved");
        return;
    }
    
    uint64_t readSize = 0;
    void* extractedData = xpkExtractData(xpk, 0, &readSize);
    if (!extractedData) {
        xpkClose(xpk);
        FAIL("ExtractData failed");
        return;
    }
    
    if (readSize != sizeof(writeData) || memcmp(writeData, extractedData, sizeof(writeData)) != 0) {
        free(extractedData);
        xpkClose(xpk);
        FAIL("Data mismatch after reopen");
        return;
    }
    
    free(extractedData);
    xpkClose(xpk);
    cleanupTestFiles("test_reopen");
    PASS();
}
int main(void)
{
    xrtInit();
    
    printf("================================================\n");
    printf(" xPack Ver7 - Raw I/O Test\n");
    printf("================================================\n");
    
    testOpenClose();
    testRawWriteRead();
    testRawOffsetWriteRead();
    testRawLargeData();
    testVolumeMapping();
    testVolumeCrossBoundary();
    testReadonlyWrite();
    testNullParams();
    testSaveAndReopen();
    
    printf("\n================================================\n");
    printf(" Results: %d tests, %d passed, %d failed\n", 
           g_testsPassed + g_testsFailed, g_testsPassed, g_testsFailed);
    printf("================================================\n");
    
    return (g_testsFailed > 0) ? 1 : 0;
}
